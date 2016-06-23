
#ifndef __GATE_SERVICE_H__
#define __GATE_SERVICE_H__

#include <unordered_map>
#include <unordered_set>
#include "ClientSession.h"
#include "serv/Service.h"
#include "../ssproto/SSMsg.h"
#include "util/DynamicFactory.h"

class GateService : public sframe::Service, public sframe::DynamicCreate<GateService>
{
public:
	GateService() : _new_session_id(1) {}
	virtual ~GateService(){}

	// ��ʼ������������ɹ�����ã���ʱ��δ��ʼ���У�
	void Init() override;

	// �����ӵ���
	void OnNewConnection(const sframe::ListenAddress & listen_addr_info, const std::shared_ptr<sframe::TcpSocket> & sock) override;

	// ����Ͽ������뱾����������Ϣ�����ķ���Ͽ�ʱ���Ż���֪ͨ��
	void OnServiceLost(const std::vector<int32_t> & services) override;

	// �Ƿ��������
	bool IsDestroyCompleted() const override
	{
		return _sessions.empty();
	}

private:
	int32_t ChooseWorkService();
	const std::string & GetLogName();

private:
	void OnMsg_SessionClosed(const std::shared_ptr<ClientSession> & session);
	void OnMsg_SessionRecvData(const std::shared_ptr<ClientSession> & session, const std::shared_ptr<std::vector<char>> & data);

	void OnMsg_RegistWorkService(int32_t work_sid);
	void OnMsg_SendToClient(const GateMsg_SendToClient & data);

private:
	int32_t _new_session_id;
	std::unordered_map<int32_t, std::shared_ptr<ClientSession>> _sessions;
	std::unordered_set<int32_t> _usable_work_service;
	std::string _log_name;
};

#endif
