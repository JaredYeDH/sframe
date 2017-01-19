
#include <string.h>
#include <algorithm>
#include <assert.h>
#include "StringHelper.h"

// �ָ��ַ���
std::vector<std::string> sframe::SplitString(const std::string & str, const std::string & sep)
{
	std::vector<std::string> result;

	if (str.size() == 0)
	{
		return result;
	}

	size_t oldPos, newPos;
	oldPos = 0;
	newPos = 0;

	while (1)
	{
		newPos = str.find(sep, oldPos);
		if (newPos != std::string::npos)
		{
			result.push_back(std::move(str.substr(oldPos, newPos - oldPos)));
			oldPos = newPos + sep.size();
		}
		else if (oldPos <= str.size())
		{
			result.push_back(std::move(str.substr(oldPos)));
			break;
		}
		else
		{
			break;
		}
	}

	return result;
}

// �����ַ����ַ���������������ִ���
int32_t sframe::GetCharMaxContinueInString(const std::string & str, char c)
{
	int32_t cur = 0;
	int32_t max = 0;

	for (size_t i = 0; i < str.length(); i++)
	{
		if (str[i] == c)
		{
			cur++;
		}
		else
		{
			max = std::max(max, cur);
			cur = 0;
		}
	}

	max = std::max(max, cur);

	return max;
}

// �����Ӵ�
int32_t sframe::FindFirstSubstr(const char * str, int32_t len, const char * sub_str)
{
	int32_t sub_str_len = strlen(sub_str);
	if (sub_str_len <= 0 || len < sub_str_len)
	{
		return -1;
	}

	for (int32_t i = 0; i < len - sub_str_len + 1; i++)
	{
		if (memcmp(str + i, sub_str, sub_str_len) == 0)
		{
			return i;
		}
	}

	return -1;
}


static const char kUpperLower = 'a' - 'Z';

// ���ַ���ת��Ϊ��д
void sframe::UpperString(std::string & str)
{
	for (std::string::iterator it = str.begin(); it != str.end(); it++)
	{
		char & c = (*it);
		if (c >= 'a' && c <= 'z')
		{
			c -= kUpperLower;
		}
	}
}

// ���ַ���ת��ΪСд
void sframe::LowerString(std::string & str)
{
	for (std::string::iterator it = str.begin(); it != str.end(); it++)
	{
		char & c = (*it);
		if (c >= 'A' && c <= 'Z')
		{
			c += kUpperLower;
		}
	}
}

// ���ַ���ת��Ϊ��д
std::string sframe::ToUpper(const std::string & str)
{
	std::string new_str;
	new_str.reserve(str.size());

	for (std::string::const_iterator it = str.begin(); it != str.end(); it++)
	{
		char c = (*it);
		if (c >= 'a' && c <= 'z')
		{
			c -= kUpperLower;
		}
		new_str.push_back(c);
	}

	return new_str;
}

// ���ַ���ת��ΪСд
std::string sframe::ToLower(const std::string & str)
{
	std::string new_str;
	new_str.reserve(str.size());

	for (std::string::const_iterator it = str.begin(); it != str.end(); it++)
	{
		char c = (*it);
		if (c >= 'A' && c <= 'Z')
		{
			c += kUpperLower;
		}
		new_str.push_back(c);
	}

	return new_str;
}

// ȥ��ͷ���մ�
std::string sframe::TrimLeft(const std::string & str)
{
	size_t pos = str.find_first_not_of(' ');
	if (pos == std::string::npos)
	{
		return "";
	}

	return str.substr(pos);
}

// ȥ��β���մ�
std::string sframe::TrimRight(const std::string & str)
{
	size_t pos = str.find_last_not_of(' ');
	if (pos == std::string::npos)
	{
		return "";
	}

	return str.substr(0, pos + 1);
}

// ȥ�����߿մ�
std::string sframe::Trim(const std::string & str)
{
	return TrimLeft(TrimRight(str));
}