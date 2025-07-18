#ifndef JSONTYPES_H
#define JSONTYPES_H

#include <cstddef>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

namespace simpleJson
{

class JsonValue;
template <typename T>
using enableIfJson =
    std::enable_if_t<(std::is_same_v<std::decay_t<T>, std::unordered_map<std::string, JsonValue>> ||
                      std::is_same_v<std::decay_t<T>, std::vector<JsonValue>> ||
                      std::is_same_v<std::decay_t<T>, std::string> || std::is_same_v<T, bool> ||
                      std::is_same_v<T, std::nullptr_t> || std::is_floating_point_v<T> || std::is_integral_v<T>)>;

// 明确json的数据类型有哪些
enum class JsonType
{
    Object,
    Array,
    String,
    Int,
    Float,
    Bool,
    Null
};

class JsonValue
{
    JsonType _curType; // 当前的json类型
    std::variant<std::unordered_map<std::string, JsonValue>, std::vector<JsonValue>, std::string, long long,
                 long double, bool, std::nullptr_t>
        _curVal;

  public:
    template <typename T, typename = enableIfJson<T>> JsonValue(T &&val)
    {

        if constexpr (std::is_same_v<std::decay_t<T>, std::unordered_map<std::string, JsonValue>>)
        {
            _curVal.emplace<std::unordered_map<std::string, JsonValue>>(std::forward<T>(val));
            _curType = JsonType::Object;
        }
        else if constexpr (std::is_same_v<std::decay_t<T>, std::vector<JsonValue>>)
        {
            _curVal.emplace<std::vector<JsonValue>>(std::forward<T>(val));
            _curType = JsonType::Array;
        }
        else if constexpr (std::is_same_v<std::decay_t<T>, std::string>)
        {
            _curVal.emplace<std::string>(std::forward<T>(val));
            _curType = JsonType::String;
        }
        else if constexpr (std::is_same_v<T, bool>)
        {
            _curVal.emplace<bool>(std::forward<T>(val));
            _curType = JsonType::Bool;
        }
        else if constexpr (std::is_same_v<T, std::nullptr_t>)
        {
            _curVal.emplace<std::nullptr_t>(std::forward<T>(val));
            _curType = JsonType::Null;
        }
        else if constexpr (std::is_integral_v<T>)
        {
            _curVal.emplace<long long>(std::forward<T>(val));
            _curType = JsonType::Int;
        }
        else if constexpr (std::is_convertible_v<T, long double>)
        {
            _curVal.emplace<long double>(std::forward<T>(val));
            _curType = JsonType::Float;
        }
    }

    template <JsonType Type> auto getval() const
    {
        if (Type != _curType)
            throw std::runtime_error("Type mismatch in getval()");

        if constexpr (Type == JsonType::Object)
            return std::get<std::unordered_map<std::string, JsonValue>>(_curVal);
        else if constexpr (Type == JsonType::Array)
            return std::get<std::vector<JsonValue>>(_curVal);
        else if constexpr (Type == JsonType::String)
            return std::get<std::string>(_curVal);
        else if constexpr (Type == JsonType::Bool)
            return std::get<bool>(_curVal);
        else if constexpr (Type == JsonType::Int)
            return std::get<long long>(_curVal);
        else if constexpr (Type == JsonType::Float)
            return std::get<long double>(_curVal);
        else if constexpr (Type == JsonType::Float)
            return std::get<std::nullptr_t>(_curVal);
    }
};

} // namespace simpleJson

#endif // JSONTYPES_H