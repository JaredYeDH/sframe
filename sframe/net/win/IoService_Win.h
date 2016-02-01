
#ifndef SFRAME_IO_SERVICE_WIN_H
#define SFRAME_IO_SERVICE_WIN_H

#include <atomic>
#include "IoUnit.h"
#include "../IoService.h"

namespace sframe {

// windows��Io����(��ɶ˿�ʵ��)
class IoService_Win : public IoService
{
public:
	IoService_Win();

	virtual ~IoService_Win();

	Error Init() override;

	void RunOnce(int32_t wait_ms, Error & err) override;

	// ע��Socket
	bool RegistSocket(const IoUnit & io_unit);

	// Ͷ����Ϣ
	void PostIoMsg(const IoMsg & io_msg);

private:
	std::atomic_bool _busy;  // �Ƿ�æµ
	HANDLE _iocp;
};

}

#endif