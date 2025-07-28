#ifndef UTILITIES_H
#define UTILITIES_H

#include <string>

namespace simple_json
{
std::string EncodeUtf8(unsigned long codepoint) noexcept; // 将unicode序列解析为utf8编码

std::string ConvertUnicodeEscape(const std::string &escape) noexcept; // 解析unicode序列

bool IsAscii(int ch) noexcept; // 判断一个字符是不是ascii字符
} // namespace simple_json

#endif // UTILITIES_H
