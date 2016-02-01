
#ifndef SFRAME_STRING_HELPER_H
#define SFRAME_STRING_HELPER_H

#include <string>
#include <vector>

namespace sframe {

// �ָ��ַ���
int32_t SplitString(const std::string & str, std::vector<std::string> & result, const std::string & sep);

// �����ַ����ַ���������������ִ���
int32_t GetCharMaxContinueInString(const std::string & str, char c);

}

#endif
