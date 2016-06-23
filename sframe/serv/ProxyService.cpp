
#include "ServiceDispatcher.h"
#include "ProxyService.h"
#include "../net/SocketAddr.h"
#include "../util/Log.h"
#include "../util/TimeHelper.h"

using namespace sframe;

ProxyService::ProxyService() 
	:  _session_num(0), _listening(false), _timer_mgr(std::bind(&ProxyService::GetServiceSessionById, this, std::placeholders::_1))
{
	memset(_session, 0, sizeof(_session));

	// ��ʼ��session_id����
	for (int i = 1; i <= kMaxSessionNumber; i++)
	{
		if (!_session_id_queue.Push(i))
		{
			assert(false);
		}
	}
}

ProxyService::~ProxyService()
{
	for (int i = 1; i <= kMaxSessionNumber; i++)
	{
		if (_session[i] != nullptr)
		{
			delete _session[i];
		}
	}
}

void ProxyService::Init()
{
	// ע����Ϣ������
	this->RegistInsideServiceMessageHandler(kProxyServiceMsgId_SessionClosed, &ProxyService::OnMsg_SessionClosed, this);
	this->RegistInsideServiceMessageHandler(kProxyServiceMsgId_SessionRecvData, &ProxyService::OnMsg_SessionRecvData, this);
	this->RegistInsideServiceMessageHandler(kProxyServiceMsgId_SessionConnectCompleted, &ProxyService::OnMsg_SessionConnectCompleted, this);
}

void ProxyService::OnDestroy()
{
	for (int i = 1; i <= kMaxSessionNumber; i++)
	{
		if (_session[i] != nullptr)
		{
			_session[i]->Close();
		}
	}
}

bool ProxyService::IsDestroyCompleted() const
{
	return (_session_num <= 0);
}

// �������ڶ�ʱ��
void ProxyService::OnCycleTimer()
{
	int64_t cur_time = sframe::TimeHelper::GetEpochMilliseconds();
	_timer_mgr.Execute(cur_time);
}

// �����ӵ���
void ProxyService::OnNewConnection(const ListenAddress & listen_addr_info, const std::shared_ptr<sframe::TcpSocket> & sock)
{
	int32_t session_id = -1;
	if (!_session_id_queue.Pop(&session_id))
	{
		sock->Close();
		LOG_INFO << "ServiceSession number upper limit, current session number: " << _session_num << ENDL;
		return;
	}

	assert(_session[session_id] == nullptr);
	_session[session_id] = new ServiceSession(session_id, this, sock);
	_session_num++;
	_session[session_id]->Init();
}

// ���������Ϣ
void ProxyService::OnProxyServiceMessage(const std::shared_ptr<ProxyServiceMessage> & msg)
{
	auto it = _remote_service_info.find(msg->dest_sid);
	if (it == _remote_service_info.end())
	{
		return;
	}

	int32_t sessionid = it->second.sessionid;
	assert(sessionid > 0 && sessionid <= kMaxSessionNumber && _session[sessionid]);
	// ��ӹ����ı��ط����Ա���session�Ͽ�ʱ��֪ͨ�ñ��ط���
	it->second.linked_local_services.insert(msg->src_sid);
	// ���÷���
	_session[sessionid]->SendData(msg);
}

#define MAKE_ADDR_INFO(ip, port) ((((int64_t)(ip) & 0xffffffff) << 16) | ((int64_t)(port) & 0xffff))

// ע��Ự
// ���ػỰID��С��0ʧ��
int32_t ProxyService::RegistSession(int32_t sid, const std::string & remote_ip, uint16_t remote_port)
{
	auto it_remote_service_info = _remote_service_info.find(sid);
	if (it_remote_service_info != _remote_service_info.end())
	{
		assert(it_remote_service_info->second.sessionid > 0 && it_remote_service_info->second.sid > 0);
		return it_remote_service_info->second.sessionid;
	}

	int32_t session_id = -1;
	SocketAddr sock_addr(remote_ip.c_str(), remote_port);
	int64_t addr_info = MAKE_ADDR_INFO(sock_addr.GetIp(), sock_addr.GetPort());
	auto it_session_id = _session_addr_to_sessionid.find(addr_info);
	if (it_session_id == _session_addr_to_sessionid.end())
	{
		// ��û����ͬĿ�ĵ�ַ��session���½�һ��
		if (!_session_id_queue.Pop(&session_id))
		{
			return -1;
		}

		assert(_session[session_id] == nullptr);
		_session[session_id] = new ServiceSession(session_id, this, remote_ip, remote_port);
		_session_num++;
		_session[session_id]->Init();
		_session_addr_to_sessionid[addr_info] = session_id;
	}
	else
	{
		session_id = it_session_id->second;
	}

	assert(session_id > 0 && session_id <= kMaxSessionNumber && _session[session_id]);
	// ��ӻỰ�����ķ���
	_sessionid_to_sid[session_id].insert(sid);
	// �����µ�Զ�̷�����Ϣ
	RemoteServiceInfo & remote_info = _remote_service_info[sid];
	remote_info.sid = sid;
	remote_info.sessionid = session_id;

	return session_id;
}

// ע��Ự��ʱ��
int32_t ProxyService::RegistSessionTimer(int32_t session_id, int32_t after_ms, sframe::ObjectTimerManager<int32_t, ServiceSession>::TimerFunc func)
{
	assert(after_ms >= 0 && session_id > 0 && session_id <= kMaxSessionNumber && _session[session_id]);
	int64_t cur_time = sframe::TimeHelper::GetEpochMilliseconds();
	return _timer_mgr.Regist(cur_time + after_ms, session_id, func);
}

ServiceSession * ProxyService::GetServiceSessionById(int32_t session_id)
{
	assert(session_id > 0 && session_id <= kMaxSessionNumber);
	return _session[session_id];
}


void ProxyService::OnMsg_SessionClosed(bool by_self, int32_t session_id)
{
	assert(_session[session_id]);

	if (!by_self)
	{
		// ֪ͨ������ı��ط���
		// ��ѯsession�����ķ���
		auto it = _sessionid_to_sid.find(session_id);
		if (it != _sessionid_to_sid.end())
		{
			std::unordered_map<int32_t, std::shared_ptr<ServiceLostMessage>> lost_msgs;

			for (auto it_remote_sid = it->second.begin(); it_remote_sid != it->second.end(); it_remote_sid++)
			{
				int32_t remote_sid = *it_remote_sid;
				auto it_remote_info = _remote_service_info.find(remote_sid);
				if (it_remote_info == _remote_service_info.end())
				{
					assert(false);
					continue;
				}

				assert(session_id == it_remote_info->second.sessionid);

				auto & local_sid_set = it_remote_info->second.linked_local_services;
				for (int32_t local_sid : local_sid_set)
				{
					auto & msg = lost_msgs[local_sid];
					if (!msg)
					{
						msg = std::make_shared<ServiceLostMessage>();
						msg->service.reserve(8);
					}
					assert(msg);
					msg->service.push_back(remote_sid);
				}
			}

			// ���η�����Ϣ
			for (auto it_msg = lost_msgs.begin(); it_msg != lost_msgs.end(); it_msg++)
			{
				int32_t sid = it_msg->first;
				assert(ServiceDispatcher::Instance().IsLocalService(sid));
				ServiceDispatcher::Instance().SendMsg(sid, it_msg->second);
			}
		}
	}

	// �Ƿ�Ҫɾ��session
	if (!_session[session_id]->TryFree())
	{
		return;
	}

	// ɾ��session
	delete _session[session_id];
	_session[session_id] = nullptr;
	_session_num--;
	if (!_session_id_queue.Push(session_id))
	{
		assert(false);
	}

	// ɾ��������¼
	auto it_sid = _sessionid_to_sid.find(session_id);
	if (it_sid != _sessionid_to_sid.end())
	{
		for (int32_t rm_sid : it_sid->second)
		{
			_remote_service_info.erase(rm_sid);
		}

		_sessionid_to_sid.erase(session_id);
	}
}

void ProxyService::OnMsg_SessionRecvData(int32_t session_id, const std::shared_ptr<std::vector<char>> & data)
{
	assert(_session[session_id]);

	std::vector<char> & vec_data = *data;
	char * p = &vec_data[0];
	uint32_t len = (uint32_t)vec_data.size();
	StreamReader reader(p, len);

	// ��ȡ��Ϣͷ��
	int32_t src_sid = 0;
	int32_t dest_sid = 0;
	uint16_t msg_id = 0;
	if (!AutoDecode(reader, src_sid, dest_sid, msg_id))
	{
		LOG_ERROR << "decode net service message error" << std::endl;
		return;
	}

	// Դ����ID�Ƿ�ͱ��ط���ID��ͻ
	if (ServiceDispatcher::Instance().IsLocalService(src_sid))
	{
		LOG_ERROR << "service message, src_id conflict with local service|" << src_sid << std::endl;
		return;
	}

	// �����Ƿ���Ŀ�����
	if (!ServiceDispatcher::Instance().IsLocalService(dest_sid))
	{
		LOG_ERROR << "service message, have no local service|" << dest_sid << std::endl;
		return;
	}

	// ����Զ�̷����¼��Ϣ
	auto it_info = _remote_service_info.find(src_sid);
	if (it_info != _remote_service_info.end())
	{
		// session �Ƿ��ͻ
		if (it_info->second.sessionid != session_id)
		{
			LOG_ERROR << "service message, session conflicted" << std::endl;
			return;
		}
	}
	else
	{
		auto insert_ret = _remote_service_info.insert(std::make_pair(src_sid, RemoteServiceInfo{}));
		assert(insert_ret.second);
		it_info = insert_ret.first;
		it_info->second.sid = src_sid;
		it_info->second.sessionid = session_id;
	}

	// ��ӹ����ı��ط����Ա���session�Ͽ�ʱ��֪ͨ�ñ��ط���
	it_info->second.linked_local_services.insert(dest_sid);
	// ��ӻỰ�����ķ�����Ϣ
	_sessionid_to_sid[it_info->second.sessionid].insert(src_sid);

	// ��װ��Ϣ�����͵�Ŀ�걾�ط���
	int32_t data_len = (int32_t)len - (int32_t)reader.GetReadedLength();
	assert(data_len >= 0);
	std::shared_ptr<NetServiceMessage> msg = std::make_shared<NetServiceMessage>();
	msg->dest_sid = dest_sid;
	msg->src_sid = src_sid;
	msg->msg_id = msg_id;
	msg->data = std::move(vec_data);
	// ���͵�Ŀ�����
	ServiceDispatcher::Instance().SendMsg(dest_sid, msg);
}

void ProxyService::OnMsg_SessionConnectCompleted(int32_t session_id, bool success)
{
	assert(_session[session_id]);
	_session[session_id]->DoConnectCompleted(success);
}
