
#include <memory.h>
#include <assert.h>
#include "ConfigSet.h"

using namespace sframe;


// ����(ȫ���ɹ�����true, ֻҪ��һ��ʧ�ܶ��᷵��false)
// ����ʱ��err_info�᷵�س����������Ϣ
bool ConfigSet::Load(const std::string & path, std::vector<ConfigError> * vec_err_info)
{
	if (path.empty() || !_config.empty())
	{
		return false;
	}

	_config_dir = path;

	std::map<int32_t, std::shared_ptr<ConfigBase>> map_conf;
	bool ret = true;

	// ����
	for (auto it = _config_load_helper.begin(); it != _config_load_helper.end(); it++)
	{
		Func_LoadConfig func_load = it->second.func_load;
		if (!func_load)
		{
			assert(false);
			continue;
		}

		std::shared_ptr<ConfigBase> conf = (this->*func_load)();
		if (!conf)
		{
			ret = false;
			if (vec_err_info)
			{
				ConfigError err;
				err.err_type = ConfigError::kLoadConfigError;
				err.config_type = it->first;
				err.config_file_name = it->second.conf_file_name;
				vec_err_info->push_back(err);
			}
		}
		else
		{
			map_conf[it->first] = conf;
		}
	}

	// ��ʼ��
	for (auto it = map_conf.begin(); it != map_conf.end(); it++)
	{
		const auto & load_helper = _config_load_helper[it->first];
		if (!it->second || !load_helper.func_init)
		{
			assert(false);
			continue;
		}

		if (!(this->*load_helper.func_init)(it->second))
		{
			ret = false;
			if (vec_err_info)
			{
				ConfigError err;
				err.err_type = ConfigError::kInitConfigError;
				err.config_type = it->first;
				err.config_file_name = load_helper.conf_file_name;
				vec_err_info->push_back(err);
			}
		}
		else
		{
			_config[it->first] = it->second;
		}
	}

	return ret;
}