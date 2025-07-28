#include "utilities.h"
#include <cctype>
#include <stdexcept>

namespace simple_json
{
std::string EncodeUtf8(const unsigned long codepoint) noexcept
{
    std::string result;
    if (codepoint <= 0x7F)
    {
        result.push_back(static_cast<char>(codepoint));
    }
    else if (codepoint <= 0x7FF)
    {
        result.push_back(static_cast<char>(0xC0 | (codepoint >> 6)));
        result.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
    }
    else if (codepoint <= 0xFFFF)
    {
        result.push_back(static_cast<char>(0xE0 | (codepoint >> 12)));
        result.push_back(static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F)));
        result.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
    }
    else if (codepoint <= 0x10FFFF)
    {
        result.push_back(static_cast<char>(0xF0 | (codepoint >> 18)));
        result.push_back(static_cast<char>(0x80 | ((codepoint >> 12) & 0x3F)));
        result.push_back(static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F)));
        result.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
    }
    else
    {
        result.append("[?]");
    }

    return result;
}

std::string ConvertUnicodeEscape(const std::string &escape) noexcept
{
    std::string result;
    result.reserve(escape.length());

    for (size_t i = 0; i < escape.length();)
    {
        if (escape[i] == '\\' && i + 5 < escape.length() && escape[i + 1] == 'u')
        {
            try
            {
                const std::string tmp = escape.substr(i + 2, 4);
                size_t failed_index = 0;
                const unsigned long codepoint = std::stoul(tmp, &failed_index, 16);
                if (failed_index == 4)
                {
                    // 如果failed_index为4，就代表std::stoul解析的过程中没有出错
                    result.append(EncodeUtf8(codepoint));
                }
                else
                {
                    // 反之不为4，那么就一定是解析出问题了
                    throw std::invalid_argument("invalid escape sequence");
                }
            }
            catch (const std::exception &e)
            {
                result.append(escape.substr(i, 6));
            }
            i += 6;
        }
        else
        {
            result.push_back(escape[i]);
            i += 1;
        }
    }

    return result;
}

bool IsAscii(int ch) noexcept
{
    return static_cast<unsigned>(ch) < 0x80;
}

} // namespace simple_json
