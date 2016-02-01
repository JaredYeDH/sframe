
#ifndef SFRAME_CONFIG_SET_H
#define SFRAME_CONFIG_SET_H

#include <string>
#include <memory>
#include <vector>
#include <unordered_map>
#include "ConfigDef.h"
#include "../util/Lock.h"

namespace sframe {

// ���ü���
class ConfigSet
{
public:

	ConfigSet(int32_t max_count);

	virtual ~ConfigSet();

	// ����
	bool Load(const std::string & dir);

	// ���¼���
	bool Reload();

	// ��ȡ����(�ڲ�ʹ��ǿ��ָ������ת�������ȷ��������ȷ)
	template<typename T>
	std::shared_ptr<const T> GetConfig(int32_t config_id)
	{
		if (config_id < 0 || config_id >= _max_count)
		{
			return nullptr;
		}

		AutoLock<SpinLock> l(_lock[config_id]);
		return std::static_pointer_cast<const T>(_config[config_id]);
	}

	// ��ȡ����(�ڲ�ʹ��ǿ��ָ������ת�������ȷ��������ȷ)
	template<typename T>
	std::shared_ptr<const T> GetConfig()
	{
		return GetConfig<T>(GET_CONFIGID(T));
	}

	// ��ȡMap��������(�ڲ�ʹ��ǿ��ָ������ת�������ȷ��������ȷ)
	template<typename T_Key, typename T_Val>
	std::shared_ptr<const std::unordered_map<T_Key, std::shared_ptr<T_Val>>> GetMapConfig(int32_t config_id)
	{
		return GetConfig<std::unordered_map<T_Key, std::shared_ptr<T_Val>>>(config_id);
	}

	// ��ȡMap��������(�ڲ�ʹ��ǿ��ָ������ת�������ȷ��������ȷ)
	template<typename T_Key, typename T_Val>
	std::shared_ptr<const std::unordered_map<T_Key, std::shared_ptr<T_Val>>> GetMapConfig()
	{
		return GetMapConfig<T_Key, T_Val>(GET_CONFIGID(T_Val));
	}

	// ��ȡMap������Ŀ(�ڲ�ʹ��ǿ��ָ������ת�������ȷ��������ȷ)
	template<typename T_Key, typename T_Val>
	std::shared_ptr<const T_Val> GetMapConfigItem(int32_t config_id, const T_Key & key)
	{
		std::shared_ptr<const std::unordered_map<T_Key, std::shared_ptr<T_Val>>> map_config = GetMapConfig<T_Key, T_Val>(config_id);
		if (map_config == nullptr)
		{
			return nullptr;
		}

		auto it = map_config->find(key);
		if (it == map_config->end())
		{
			return nullptr;
		}

		return it->second;
	}

	// ��ȡMap������Ŀ(�ڲ�ʹ��ǿ��ָ������ת�������ȷ��������ȷ)
	template<typename T_Key, typename T_Val>
	std::shared_ptr<const T_Val> GetMapConfigItem(const T_Key & key)
	{
		return GetMapConfigItem<T_Key, T_Val>(GET_CONFIGID(T_Val), key);
	}

	// ��ȡVector���͵�����(�ڲ�ʹ��ǿ��ָ������ת�������ȷ��������ȷ)
	template<typename T>
	std::shared_ptr<const std::vector<std::shared_ptr<T>>> GetVectorConfig(int32_t config_id)
	{
		return GetConfig<std::vector<std::shared_ptr<T>>>(config_id);
	}

	// ��ȡVector���͵�����(�ڲ�ʹ��ǿ��ָ������ת�������ȷ��������ȷ)
	template<typename T>
	std::shared_ptr<const std::vector<std::shared_ptr<T>>> GetVectorConfig()
	{
		return GetVectorConfig<T>(GET_CONFIGID(T));
	}

	// ��ȡVector���͵�������Ŀ(�ڲ�ʹ��ǿ��ָ������ת�������ȷ��������ȷ)
	template<typename T>
	std::shared_ptr<const std::vector<std::shared_ptr<T>>> GetVectorConfigItem(int32_t config_id, int32_t index)
	{
		std::shared_ptr<const std::vector<std::shared_ptr<T>>> v_config = GetVectorConfig<T>(config_id);
		if (v_config == nullptr || index < 0 || index >= v_config->size())
		{
			return nullptr;
		}

		return (*v_config)[index];
	}

	// ��ȡVector���͵�������Ŀ(�ڲ�ʹ��ǿ��ָ������ת�������ȷ��������ȷ)
	template<typename T>
	std::shared_ptr<const std::vector<std::shared_ptr<T>>> GetVectorConfigItem(int32_t index)
	{
		return GetVectorConfigItem(GET_CONFIGID(T), index);
	}

protected:

	// ����(������ʵ�ִ˺�������ʵ���������õļ���)
	virtual bool LoadAll() = 0;

	// ����
	template<typename T_ConfigLoader, typename T>
	std::shared_ptr<T> LoadConfig(int32_t config_id, const std::string & file_name, T_ConfigLoader & loader)
	{
		std::shared_ptr<T> o;
		if (config_id < 0 || config_id >= _max_count || _temp[config_id] != nullptr ||
			(o = std::make_shared<T>()) == nullptr || !loader.Load(_config_dir + file_name, *(o.get())))
		{
			return nullptr;
		}

		_temp[config_id] = o;

		return o;
	}

	// ����
	template<typename T_ConfigLoader, typename T>
	std::shared_ptr<T> LoadConfig(T_ConfigLoader & loader)
	{
		return LoadConfig<T_ConfigLoader, T>(GET_CONFIGID(T), GET_CONFIGFILENAME(T), loader);
	}

	// ����
	template<typename T_ConfigLoader, typename T>
	std::shared_ptr<T> LoadConfig()
	{
		T_ConfigLoader loader;
		return LoadConfig<T_ConfigLoader, T>(loader);
	}

	// ����Map
	template<typename T_ConfigLoader, typename T_Key, typename T_Val>
	std::shared_ptr<std::unordered_map<T_Key, std::shared_ptr<T_Val>>> LoadMap(int32_t config_id, const std::string & file_name, T_ConfigLoader & loader)
	{
		return LoadConfig<T_ConfigLoader, std::unordered_map<T_Key, std::shared_ptr<T_Val>>>(config_id, file_name, loader);
	}

	// ����Map
	template<typename T_ConfigLoader, typename T_Key, typename T_Val>
	std::shared_ptr<std::unordered_map<T_Key, std::shared_ptr<T_Val>>> LoadMap(T_ConfigLoader & loader)
	{
		return LoadMap<T_ConfigLoader, T_Key, T_Val>(GET_CONFIGID(T_Val), GET_CONFIGFILENAME(T_Val), loader);
	}

	// ����Map
	template<typename T_ConfigLoader, typename T_Key, typename T_Val>
	std::shared_ptr<std::unordered_map<T_Key, std::shared_ptr<T_Val>>> LoadMap()
	{
		T_ConfigLoader loader;
		return LoadMap<T_ConfigLoader, T_Key, T_Val>(loader);
	}

	// ����Vector
	template<typename T_ConfigLoader, typename T>
	std::shared_ptr<std::vector<std::shared_ptr<T>>> LoadVector(int32_t config_id, const std::string & file_name, T_ConfigLoader & loader)
	{
		return LoadConfig<T_ConfigLoader, std::vector<std::shared_ptr<T>>>(config_id, file_name, loader);
	}

	// ����Vector
	template<typename T_ConfigLoader, typename T>
	std::shared_ptr<std::vector<std::shared_ptr<T>>> LoadVector(T_ConfigLoader & loader)
	{
		return LoadVector<T_ConfigLoader, T>(GET_CONFIGID(T), GET_CONFIGFILENAME(T), loader);
	}

	// ����Vector
	template<typename T_ConfigLoader, typename T>
	std::shared_ptr<std::vector<std::shared_ptr<T>>> LoadVector()
	{
		T_ConfigLoader loader;
		return LoadVector<T_ConfigLoader, T>(loader);
	}

private:
	// Ӧ�����ã�����ʱ�������滻����ʽ���ã�
	void Apply();

private:
	SpinLock * _lock;                     // ÿһ������һ��������
	std::shared_ptr<void> * _config;      // ��������
	std::shared_ptr<void> * _temp;        // ��ʱ��
	int32_t _max_count;                   // �������
	std::string _config_dir;              // �����ļ�Ŀ¼
};

}

#endif
