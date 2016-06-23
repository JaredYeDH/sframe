
#include "ServiceDispatcher.h"
#include "ServiceSession.h"
#include "ProxyService.h"
#include "../util/Log.h"
#include "../util/TimeHelper.h"
#include "../util/md5.h"

using namespace sframe;

ServiceSession::ServiceSession(int32_t id, ProxyService * proxy_service, const std::string & remote_ip, uint16_t remote_port)
	: _session_id(id), _remote_ip(remote_ip), _remote_port(remote_port), _proxy_service(proxy_service), 
	_state(kSessionState_WaitConnect),  _reconnect(true)
{
	assert(!remote_ip.empty() && proxy_service);
}

ServiceSession::ServiceSession(int32_t id, ProxyService * proxy_service, const std::shared_ptr<sframe::TcpSocket> & sock)
	: _session_id(id), _socket(sock), _state(kSessionState_Running), _proxy_service(proxy_service), _reconnect(false)
{
	assert(sock != nullptr && proxy_service);
}


void ServiceSession::Init()
{
	if (!_socket)
	{
		_socket = sframe::TcpSocket::Create(ServiceDispatcher::Instance().GetIoService());
		_socket->SetMonitor(this);
		StartConnectTimer(0);
	}
	else
	{
		_socket->SetMonitor(this);
		// ��ʼ��������
		_socket->StartRecv();
	}
}


void ServiceSession::Close()
{
	bool send_close = true;

	_reconnect = false;

	if (_socket != nullptr)
	{
		send_close = _socket->Close() == false;
	}

	if (send_close)
	{
		bool by_self = true;
		std::shared_ptr<InsideServiceMessage<bool, int32_t>> msg = std::make_shared<InsideServiceMessage<bool, int32_t>>(by_self, _session_id);
		msg->dest_sid = 0;
		msg->src_sid = 0;
		msg->msg_id = kProxyServiceMsgId_SessionClosed;
		ServiceDispatcher::Instance().SendMsg(0, msg);
	}
}

// �����ͷ�
bool ServiceSession::TryFree()
{
	if (!_reconnect)
	{
		return true;
	}

	_state = kSessionState_WaitConnect;
	// ȫ�´���һ��socket
	_socket = sframe::TcpSocket::Create(ServiceDispatcher::Instance().GetIoService());
	_socket->SetMonitor(this);
	// �������Ӷ�ʱ��
	StartConnectTimer(kReconnectInterval);

	return false;
}

// ���Ӳ�����ɴ���
void ServiceSession::DoConnectCompleted(bool success)
{
	if (!success)
	{
		// �´���һ��socket
		_socket = sframe::TcpSocket::Create(ServiceDispatcher::Instance().GetIoService());
		_socket->SetMonitor(this);
		_state = kSessionState_WaitConnect;
		// �������Ӷ�ʱ��
		StartConnectTimer(kReconnectInterval);
		return;
	}

	// ��ʼ�Ự
	_state = ServiceSession::kSessionState_Running;
	assert(_socket->IsOpen());

	// ֮ǰ����������������ͳ�ȥ
	char data[65536];
	int32_t len = 65536;
	for (auto & msg : _msg_cache)
	{
		if (msg->Serialize(data, &len))
		{
			_socket->Send(data, len);
		}
		else
		{
			LOG_ERROR << "Serialize mesage error" << std::endl;
		}
	}
	_msg_cache.clear();
}

// ��������
void ServiceSession::SendData(const std::shared_ptr<ProxyServiceMessage> & msg)
{
	if (_state != ServiceSession::kSessionState_Running)
	{
		// ��������
		_msg_cache.push_back(msg);
	}
	else
	{
		assert(_socket);
		// ֱ�ӷ���
		char data[65536];
		int32_t len = 65536;
		if (msg->Serialize(data, &len))
		{
			_socket->Send(data, len);
		}
		else
		{
			LOG_ERROR << "Serialize mesage error" << std::endl;
		}
	}
}

// ���յ�����
// ����ʣ���������
int32_t ServiceSession::OnReceived(char * data, int32_t len)
{
	assert(data && len > 0 && _state == kSessionState_Running);

	char * p = data;
	int32_t surplus = len;

	while (true)
	{
		uint16_t msg_size = 0;
		StreamReader msg_size_reader(p, surplus);
		if (!AutoDecode(msg_size_reader, msg_size) ||
			surplus - msg_size_reader.GetReadedLength() < msg_size)
		{
			break;
		}

		p += msg_size_reader.GetReadedLength();
		surplus -= msg_size_reader.GetReadedLength();

		std::shared_ptr<std::vector<char>> data = std::make_shared<std::vector<char>>(msg_size);
		if (msg_size > 0)
		{
			memcpy(&(*data)[0], p, msg_size);
			p += msg_size;
			surplus -= msg_size;
		}
			
		ServiceDispatcher::Instance().SendInsideServiceMsg(0, 0, kProxyServiceMsgId_SessionRecvData, _session_id, data);
	}

	return surplus;
}

// Socket�ر�
// by_self: true��ʾ��������Ĺرղ���
void ServiceSession::OnClosed(bool by_self, sframe::Error err)
{
	if (err)
	{
		LOG_INFO << "Connection with server(" << SocketAddrText(_socket->GetRemoteAddress()).Text() 
			<< ") closed with error(" << err.Code() << "): " << sframe::ErrorMessage(err).Message() << ENDL;
	}
	else
	{
		LOG_INFO << "Connection with server(" << SocketAddrText(_socket->GetRemoteAddress()).Text() << ") closed" << ENDL;
	}

	std::shared_ptr<InsideServiceMessage<bool, int32_t>> msg = std::make_shared<InsideServiceMessage<bool, int32_t>>(by_self, _session_id);
	msg->dest_sid = 0;
	msg->src_sid = 0;
	msg->msg_id = kProxyServiceMsgId_SessionClosed;
	ServiceDispatcher::Instance().SendMsg(0, msg);
}

// ���Ӳ������
void ServiceSession::OnConnected(sframe::Error err)
{
	bool success = true;

	if (err)
	{
		success = false;
		LOG_ERROR << "Connect to server(" << _remote_ip << ":" << _remote_port << ") error(" << err.Code() << "): " << sframe::ErrorMessage(err).Message() << ENDL;
	}
	else
	{
		LOG_INFO << "Connect to server(" << _remote_ip << ":" << _remote_port << ") success" << ENDL;
	}

	// ֪ͨ�������
	ServiceDispatcher::Instance().SendInsideServiceMsg(0, 0, kProxyServiceMsgId_SessionConnectCompleted, _session_id, success);
	// ��ʼ��������
	if (success)
	{
		_socket->StartRecv();
	}
}

// ��ʼ���Ӷ�ʱ��
void ServiceSession::StartConnectTimer(int32_t after_ms)
{
	_proxy_service->RegistSessionTimer(_session_id, after_ms, &ServiceSession::OnTimer_Connect);
}

// ��ʱ������
int32_t ServiceSession::OnTimer_Connect(int64_t cur_ms)
{
	assert(_state == kSessionState_WaitConnect && _socket && !_remote_ip.empty());

	LOG_INFO << "Start connect to server(" << _remote_ip << ":" << _remote_port << ")" << ENDL;


	_state = kSessionState_Connecting;
	_socket->Connect(sframe::SocketAddr(_remote_ip.c_str(), _remote_port));

	// ִֻ��һ�κ�ֹͣ
	return -1;
}