
#ifndef SFRAME_SERVICE_SESSION_H
#define SFRAME_SERVICE_SESSION_H

#include <list>
#include "../util/Serialization.h"
#include "../net/net.h"
#include "../util/Singleton.h"
#include "../util/Timer.h"

namespace sframe {

class ProxyService;

// ����Ự����Ҫ�����������еķ����ͨ�ţ�
class ServiceSession : public TcpSocket::Monitor, public noncopyable, public SafeTimerRegistor<ServiceSession>
{
public:
	// �Ự״̬
	enum SessionState : int32_t
	{
		kSessionState_WaitConnect = 0,   // �ȴ�����
		kSessionState_Connecting,        // ��������
		kSessionState_Running,           // ������
	};

	static const int32_t kReconnectInterval = 5000;       // �������
	static const int32_t kMaxCacheMessageNumber = 1024;   // ��໺����ٸ���Ϣ

public:
	ServiceSession(int32_t id, ProxyService * proxy_service, const std::string & remote_ip, uint16_t remote_port);

	ServiceSession(int32_t id, ProxyService * proxy_service, const std::shared_ptr<TcpSocket> & sock);

	~ServiceSession(){}

	void Init();

	// �ر�
	void Close();

	// �����ͷ�
	bool TryFree();

	// ������ɴ���
	void DoConnectCompleted(bool success);

	// ��������
	void SendData(const std::shared_ptr<ProxyServiceMessage> & msg);

	// ��ȡ״̬
	SessionState GetState() const
	{
		return _state;
	}

	// ���յ�����
	// ����ʣ���������
	int32_t OnReceived(char * data, int32_t len) override;

	// Socket�ر�
	// by_self: true��ʾ��������Ĺرղ���
	void OnClosed(bool by_self, Error err) override;

	// ���Ӳ������
	void OnConnected(Error err) override;

private:

	// ��ʼ���Ӷ�ʱ��
	void StartConnectTimer(int32_t after_ms);

	// ��ʱ������
	int64_t OnTimer_Connect();

private:
	ProxyService * _proxy_service;
	std::shared_ptr<TcpSocket> _socket;
	int32_t _session_id;
	SessionState _state;
	std::list<std::shared_ptr<ProxyServiceMessage>> _msg_cache;
	bool _reconnect;
	std::string _remote_ip;
	uint16_t _remote_port;
};

}

#endif
