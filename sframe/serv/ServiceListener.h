
#ifndef SFRAME_SERVICE_LISTENER_H
#define SFRAME_SERVICE_LISTENER_H

#include <string>
#include "../net/net.h"
#include "../util/Singleton.h"

namespace sframe {

// �������������������Զ�̷������ӣ�
class ServiceListener : public TcpAcceptor::Monitor, public noncopyable
{
public:
	ServiceListener() : _listen_port(0) {}

	~ServiceListener() {}

	// ���ü�����ַ
	void SetListenAddr(const std::string & listen_ip, uint16_t listen_port)
	{
		_listen_ip = listen_ip;
		_listen_port = listen_port;
	}

	// ��ʼ
	bool Start();

	// ֹͣ
	void Stop();

	// ����֪ͨ
	void OnAccept(std::shared_ptr<TcpSocket> socket, Error err) override;

	// ֹͣ
	void OnClosed(Error err) override;

private:
	std::shared_ptr<TcpAcceptor> _acceptor;
	std::string _listen_ip;
	uint16_t _listen_port;
};

}

#endif
