#include "json_type.h"
#include <iostream>

namespace simpleJson
{
std::ostream &operator<<(std::ostream &os, const JsonType &type)
{
    switch (type)
    {
    case JsonType::Object:
        os << "Json Object";
        break;
    case JsonType::Array:
        os << "Json Array";
        break;
    case JsonType::String:
        os << "Json String";
        break;
    case JsonType::Int:
        os << "Json Int";
        break;
    case JsonType::Float:
        os << "Json Float";
        break;
    case JsonType::Bool:
        os << "Json Bool";
        break;
    case JsonType::Null:
        os << "Json Null";
        break;
    }
    return os;
}

std::ostream &operator<<(std::ostream &os, const JsonValue &val)
{
    // json对象和数组底层分别是哈希表和vector，这里我们就不打印出来了
    const auto jsonObjectInfo = "Json Type: Json Object";
    const auto jsonArrayInfo = "Json Type: Json Array";

    switch (val._curType)
    {
    case JsonType::Object:
        os << jsonObjectInfo;
        break;
    case JsonType::Array:
        os << jsonArrayInfo;
        break;
    case JsonType::String:
        os << std::get<std::string>(val._curVal);
        break;
    case JsonType::Bool:
        os << std::get<bool>(val._curVal);
        break;
    case JsonType::Int:
        os << std::get<long long>(val._curVal);
        break;
    case JsonType::Float:
        os << std::get<long double>(val._curVal);
        break;
    case JsonType::Null:
        os << std::get<std::nullptr_t>(val._curVal);
        break;
    }
    return os;
}
} // namespace simpleJson
