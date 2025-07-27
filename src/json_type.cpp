#include "json_type.h"
#include <iostream>

namespace simple_json
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
    const auto json_object_info = "Json Type: Json Object";
    const auto json_array_info = "Json Type: Json Array";

    switch (val.cur_type_)
    {
    case JsonType::Object:
        os << json_object_info;
        break;
    case JsonType::Array:
        os << json_array_info;
        break;
    case JsonType::String:
        os << std::get<std::string>(val.cur_val_);
        break;
    case JsonType::Bool:
        os << std::get<bool>(val.cur_val_);
        break;
    case JsonType::Int:
        os << std::get<long long>(val.cur_val_);
        break;
    case JsonType::Float:
        os << std::get<long double>(val.cur_val_);
        break;
    case JsonType::Null:
        os << std::get<std::nullptr_t>(val.cur_val_);
        break;
    }
    return os;
}
} // namespace simple_json
