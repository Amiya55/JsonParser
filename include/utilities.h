#ifndef UTILITIES_H
#define UTILITIES_H

#include <string>

namespace simple_json
{
std::string EncodeUtf8(unsigned long codepoint) noexcept;

std::string ConvertUnicodeEscape(const std::string &escape) noexcept;
} // namespace simple_json

#endif // UTILITIES_H
