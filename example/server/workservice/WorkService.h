
#ifndef __WORK_SERVICE_H__
#define __WORK_SERVICE_H__

#include <unordered_map>
#include "serv/Service.h"
#include "User.h"
#include "util/DynamicFactory.h"

class WorkService : public sframe::Service, public sframe::DynamicCreate<WorkService>
{
public:
	WorkService() {}
	virtual ~WorkService() {}

	// ��ʼ������������ɹ�����ã���ʱ��δ��ʼ���У�
	void Init() override;

private:
	void OnMsg_EnterWorkService(int32_t gate_sid, int64_t session_id);

	void OnMsg_QuitWorkService(int32_t gate_sid, int64_t session_id);

	const std::string & GetLogName();

	User * GetUser(int64_t session_id);

private:
	std::unordered_map<int64_t, std::shared_ptr<User>> _users;
	std::string _log_name;
};

#endif
