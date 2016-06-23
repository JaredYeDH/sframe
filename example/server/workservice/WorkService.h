
#ifndef __WORK_SERVICE_H__
#define __WORK_SERVICE_H__

#include <unordered_map>
#include "serv/Service.h"
#include "../ssproto/SSMsg.h"
#include "User.h"
#include "util/DynamicFactory.h"

class WorkService : public sframe::Service, public sframe::DynamicCreate<WorkService>
{
public:
	WorkService() {}
	virtual ~WorkService() {}

	// ��ʼ������������ɹ�����ã���ʱ��δ��ʼ���У�
	void Init() override;

	// ����Ͽ�
	void OnServiceLost(const std::vector<int32_t> & sid_set) override;

private:
	void OnMsg_ClientData(const WorkMsg_ClientData & msg);

	void OnMsg_EnterWorkService(int32_t gate_sid, int32_t session_id);

	void OnMsg_QuitWorkService(int32_t gate_sid, int32_t session_id);

	const std::string & GetLogName();

private:
	std::unordered_map<uint64_t, std::shared_ptr<User>> _users;
	std::string _log_name;
};

#endif
