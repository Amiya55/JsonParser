#include "json.h"
#include "json_type.h"

namespace simple_json
{

Json Json::JsonObj() noexcept
{
    return Json("{}");
}

Json Json::JsonArr() noexcept
{
    return Json("[]");
}

// 匿名命名空间, 函数不对外暴露
namespace
{

// 辅助函数, 打印json缩进
void PrintIndent(std::ostream &os, size_t indent_level, size_t indent_size)
{
    const std::string indent_unit(indent_size, ' ');
    for (size_t i = 0; i < indent_level; ++i)
    {
        os << indent_unit;
    }
}

// 辅助函数, 打印json数据结构
std::ostream &PrintJson(std::ostream &os, const JsonValue &json_val, size_t indent_level, size_t indent_size)
{
    switch (json_val.GetType())
    {
    case JsonType::Object:
        os << "{\n";
        {
            size_t index = 0;
            for (const auto &pair : json_val.GetVal<JsonType::Object>())
            {
                PrintIndent(os, indent_level + 1, indent_size);
                os << '\"' << pair.first << "\": ";
                PrintJson(os, pair.second, indent_level + 1, indent_size);

                if (index != json_val.GetVal<JsonType::Object>().size() - 1)
                {
                    os << ",\n";
                }

                ++index;
            }
        }
        os << '\n';
        PrintIndent(os, indent_level, indent_size);
        os << '}';
        break;
    case JsonType::Array:
        os << "\n";
        {
            size_t index = 0;
            for (const auto &item : json_val.GetVal<JsonType::Array>())
            {
                PrintIndent(os, indent_level + 1, indent_size);
                PrintJson(os, item, indent_level + 1, indent_size);
                os << ",\n";

                ++index;
            }
        }
        os << '\n';
        PrintIndent(os, indent_level, indent_size);
        os << ']';
        break;
    case JsonType::String:
        os << '\"' << json_val.GetVal<JsonType::String>() << '\"';
        break;
    case JsonType::Int:
        os << json_val.GetVal<JsonType::Int>();
        break;
    case JsonType::Float:
        os << json_val.GetVal<JsonType::Float>();
        break;
    case JsonType::Bool:
        os << (json_val.GetVal<JsonType::Bool>() ? "true" : "false");
        break;
    case JsonType::Null:
        os << "null";
        break;
    }

    return os;
}

} // namespace

std::ostream &operator<<(std::ostream &os, const Json &json)
{
    return PrintJson(os, json.data_, 0, 2);
}

} // namespace simple_json