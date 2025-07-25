#ifndef UTILITIES_H
#define UTILITIES_H

#include <string>

namespace simpleJson
{
std::string encode_utf8(unsigned long codepoint) noexcept;

std::string convert_unicode_escape(const std::string &escape) noexcept;
} // namespace simpleJson

#endif // UTILITIES_H
