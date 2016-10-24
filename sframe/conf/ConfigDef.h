
#ifndef SFRAME_CONFIG_DEF_H
#define SFRAME_CONFIG_DEF_H

#include "ConfigSet.h"
#include "TableReader.h"
#include "JsonReader.h"

//////////////////// ��̬����������ظ���

// ������һ����ģ�͵�����
// STATIC_OBJ_CONFIG(�ṹ����, ����ID)
#define STATIC_OBJ_CONFIG(S, conf_id) \
	struct ConfigMeta \
	{ \
		static int GetConfigId() {return conf_id;} \
		typedef S ModelType; \
	};

// ����Vetorģ�͵�����
// STATIC_VECTOR_CONFIG(�ṹ����, ����ID)
#define STATIC_VECTOR_CONFIG(S, conf_id) \
	struct ConfigMeta \
	{ \
		static int GetConfigId() {return conf_id;} \
		typedef std::vector<S> ModelType; \
	};

// ����Setģ�͵�����
// STATIC_SET_CONFIG(�ṹ����, ����ID)
#define STATIC_SET_CONFIG(S, conf_id) \
	struct ConfigMeta \
	{ \
		static int GetConfigId() {return conf_id;} \
		typedef std::set<S> ModelType; \
	};

// ����Mapģ�͵�����
// STATIC_MAP_CONFIG(key����, �ṹ����, �ṹ��������key�ĳ�Ա������, ����ID)
#define STATIC_MAP_CONFIG(k_type, S, k_field_name, conf_id) \
	struct ConfigMeta \
	{ \
		static int GetConfigId() {return conf_id;} \
		typedef k_type KeyType; \
		typedef std::map<k_type, std::shared_ptr<S>> ModelType; \
		static k_type GetKey(const S & conf) { return conf.k_field_name; } \
	};


//////////////////// ��̬����������ظ���

// ������һ����ģ�͵�����
// DYNAMIC_OBJ_CONFIG(�ṹ����, ����ID)
#define DYNAMIC_OBJ_CONFIG(S, conf_id) \
	struct DynamicConfigMeta \
	{ \
		static int GetConfigId() {return conf_id;} \
		typedef S ModelType; \
	};

// ����Vetorģ�͵�����
// DYNAMIC_VECTOR_CONFIG(�ṹ����, ����ID)
#define DYNAMIC_VECTOR_CONFIG(S, conf_id) \
	struct DynamicConfigMeta \
	{ \
		static int GetConfigId() {return conf_id;} \
		typedef std::vector<S> ModelType; \
	};

// ����Setģ�͵�����
// DYNAMIC_SET_CONFIG(�ṹ����, ����ID)
#define DYNAMIC_SET_CONFIG(S, conf_id) \
	struct DynamicConfigMeta \
	{ \
		static int GetConfigId() {return conf_id;} \
		typedef std::set<S> ModelType; \
	};

// ����Mapģ�͵�����
// DYNAMIC_MAP_CONFIG(key����, �ṹ����, �ṹ��������key�ĳ�Ա������, ����ID)
#define DYNAMIC_MAP_CONFIG(k_type, S, k_field_name, conf_id) \
	struct DynamicConfigMeta \
	{ \
		static int GetConfigId() {return conf_id;} \
		typedef k_type KeyType; \
		typedef std::map<k_type, std::shared_ptr<S>> ModelType; \
		static k_type GetKey(const S & conf) { return conf.k_field_name; } \
	};

#endif
