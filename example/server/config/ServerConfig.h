
#ifndef PUBDEF_SERVER_CONFIG_H
#define PUBDEF_SERVER_CONFIG_H

#include <inttypes.h>
#include <string>
#include <vector>
#include "conf/ConfigDef.h"
#include "util/Singleton.h"

JSONCONFIG(ServerInfo)
{
	std::string ip;
	uint16_t port;
	std::string key;
};

JSONCONFIG(ServerConfig) : public sframe::singleton<ServerConfig>
{
	bool Load(const std::string & filename);

	std::string res_path;       // ��ԴĿ¼
	int32_t thread_num;         // �߳�����
	std::vector<int32_t> local_service;  // ���ؿ�������
	ServerInfo service_listen;    // ���������ַ
	ServerInfo client_listen;     // �ͻ��˼�����ַ
	std::vector<ServerInfo> remote_server;   // Ҫ���ӵ�Զ�̷�����
};


#endif
