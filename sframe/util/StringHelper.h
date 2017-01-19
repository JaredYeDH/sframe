
#ifndef SFRAME_STRING_HELPER_H
#define SFRAME_STRING_HELPER_H

#include <string>
#include <vector>

namespace sframe {

// �ָ��ַ���
std::vector<std::string> SplitString(const std::string & str, const std::string & sep);

// �����ַ����ַ���������������ִ���
int32_t GetCharMaxContinueInString(const std::string & str, char c);

// �����Ӵ�
int32_t FindFirstSubstr(const char * str, int32_t len, const char * sub_str);

// ���ַ���ת��Ϊ��д
void UpperString(std::string & str);

// ���ַ���ת��ΪСд
void LowerString(std::string & str);

// ���ַ���ת��Ϊ��д
std::string ToUpper(const std::string & str);

// ���ַ���ת��ΪСд
std::string ToLower(const std::string & str);

// ȥ��ͷ���մ�
std::string TrimLeft(const std::string & str);

// ȥ��β���մ�
std::string TrimRight(const std::string & str);

// ȥ�����߿մ�
std::string Trim(const std::string & str);

}

#endif
