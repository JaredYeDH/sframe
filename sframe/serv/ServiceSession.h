
#ifndef SFRAME_SERVICE_SESSION_H
#define SFRAME_SERVICE_SESSION_H

#include "../util/Serialization.h"
#include "../net/net.h"
#include "../util/Singleton.h"

namespace sframe {

class ProxyService;

// ����Ự����Ҫ�����������еķ����ͨ�ţ�
class ServiceSession : public TcpSocket::Monitor, public noncopyable
{
public:
	// �Ự״̬
	enum SessionState : int32_t
	{
		kSessionState_WaitConnect = 0,   // �ȴ�����
		kSessionState_Connecting,        // ��������
		kSessionState_Authing,           // ������֤(�����������ӷ��ȴ��Է�����֤���)
		kSessionState_WaitAuth,          // �ȴ���֤(�������ӷ��ȴ��Է�������֤��Ϣ)
		kSessionState_Running,           // ������
	};

	static const int32_t kReconnectInterval = 5000;    // �������

public:
	ServiceSession(int32_t id, ProxyService * proxy_service, const std::string & remote_ip, uint16_t remote_port, const std::string & remote_key);

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
	void DoRecvData(std::vector<char> & data);

	// ��������
	void SendData(const char * data, int32_t len);

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
	// ����״̬�������ݴ���
	void ReceiveData_Running(std::vector<char> & data);

	// ��֤״̬�������ݴ���
	void ReceiveData_Authing(std::vector<char> & data);

	// �ȴ���֤״̬�������ݴ���
	void ReceiveData_WaitAuth(std::vector<char> & data);

	// ������֤��Ϣ
	bool SendAuthMessage();

	// ������֤�����Ϣ
	bool SendAuthCompletedMessage(bool success);

	// ��ʼ�Ự
	void Start(const std::vector<int32_t> & remote_service);

	// ��ʼ���Ӷ�ʱ��
	void StartConnectTimer(int32_t after_ms);

	// ��ʱ������
	int32_t OnTimer_Connect(int64_t cur_ms);

private:
	ProxyService * _proxy_service;
	std::shared_ptr<TcpSocket> _socket;
	int32_t _session_id;
	SessionState _state;
	bool _reconnect;
	// ����3����Ա���������������������ӷ�
	std::string _remote_ip;
	uint16_t _remote_port;
	std::string _remote_key;
};

}

#endif
