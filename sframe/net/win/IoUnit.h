
#ifndef SFRAME_IO_UNIT_H
#define SFRAME_IO_UNIT_H

#include <inttypes.h>
#include <memory>
#include <WinSock2.h>
#include "../Error.h"

namespace sframe {

// IO�¼�����
enum IoEventType :int32_t
{
	kIoEvent_ConnectCompleted,      // ����
	kIoEvent_SendCompleted,         // ����
	kIoEvent_RecvCompleted,         // ��������
	kIoEvent_AcceptCompleted,       // ��������
};

class IoUnit;

// IO�¼�
struct IoEvent
{
	IoEvent(IoEventType t)
		: evt_type(t), err(kErrorCode_Success)
	{
		memset(&ol, 0, sizeof(ol));
	}

	OVERLAPPED ol;
	const IoEventType evt_type;   // �¼�����
	std::shared_ptr<IoUnit> io_unit; // ���ڹ���IoUnit����IO�����ô˽�������¼�֪ͨ��ͬʱ��֤��һ�β������֮ǰ���󲻻ᱻ������
	Error err;                    // ������
	int32_t data_len;             // ���ݳ���
};

// IO��Ϣ����
enum IoMsgType : int32_t
{
	kIoMsgType_Close,        // �ر�
	kIoMsgType_NotifyError,  // ����֪ͨ
};

// IO��Ϣ
struct IoMsg
{
	IoMsg(IoMsgType t) : msg_type(t) {}

	IoMsgType msg_type;
	std::shared_ptr<IoUnit> io_unit;
};

class IoService;

// Io��Ԫ
class IoUnit
{
public:
	IoUnit(const std::shared_ptr<IoService> & io_service) : _sock(INVALID_SOCKET), _io_service(io_service) {}

	virtual ~IoUnit()
	{
		if (_sock != INVALID_SOCKET)
		{
			closesocket(_sock);
		}
	}

	virtual void OnEvent(IoEvent * io_evt) = 0;

	virtual void OnMsg(IoMsg * io_msg) = 0;

	SOCKET GetSocket() const
	{
		return _sock;
	}

protected:
	SOCKET _sock;
	std::shared_ptr<IoService> _io_service;
};

}

#endif