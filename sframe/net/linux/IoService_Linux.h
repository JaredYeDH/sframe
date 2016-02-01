
#ifndef SFRAME_IO_SERVICE_LINUX_H
#define SFRAME_IO_SERVICE_LINUX_H

#include <atomic>
#include <vector>
#include "../IoService.h"
#include "IoUnit.h"
#include "../../util/Lock.h"

namespace sframe {

// Linux�µ�Io����(����EPOLLʵ��)
class IoService_Linux : public IoService
{
public:
	// epoll�ȴ�����¼�����
	static const int kMaxEpollEventsNumber = 1024;
	// IO��Ϣ����������
	static const int kMaxIoMsgBufferSize = 65536;

public:
	IoService_Linux();

	virtual ~IoService_Linux();

	Error Init() override;

	void RunOnce(int32_t wait_ms, Error & err) override;

	// ��Ӽ����¼�
	bool AddIoEvent(const IoUnit & iounit, const IoEvent ioevt);

	// �޸ļ����¼�
	bool ModifyIoEvent(const IoUnit & iounit, const IoEvent ioevt);

	// ɾ�������¼�
	bool DeleteIoEvent(const IoUnit & iounit, const IoEvent ioevt);

	// Ͷ����Ϣ
	void PostIoMsg(const IoMsg & io_msg);

private:
	std::atomic_bool _busy;
	int _epoll_fd;
	int _msg_evt_fd;               // ����ʵ��IO��Ϣ�ķ����봦��
	std::vector<IoMsg*> _msgs;     // IO��Ϣ�б�
	sframe::Lock _msgs_lock;  // ��Ϣ�б���
};

}

#endif
