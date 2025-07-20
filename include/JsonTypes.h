#ifndef JSON_TYPES_H
#define JSON_TYPES_H

#include <cstddef>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>
#include <initializer_list>
#include "Config.h"

namespace simpleJson {
    class JsonValue;
    template<typename T>
    using enableIfJson =
    std::enable_if_t<std::is_same_v<std::decay_t<T>, std::unordered_map<std::string, JsonValue> > ||
                     std::is_same_v<std::decay_t<T>, std::vector<JsonValue> > ||
                     std::is_same_v<std::decay_t<T>, std::string> ||
                     std::is_same_v<std::decay_t<T>, const char *> ||
                     std::is_same_v<std::decay_t<T>, bool> ||
                     std::is_same_v<std::decay_t<T>, std::nullptr_t> ||
                     std::is_floating_point_v<T> ||
                     std::is_integral_v<T>>;

    // 明确json的数据类型有哪些
    enum class JsonType {
        Object,
        Array,
        String,
        Int,
        Float,
        Bool,
        Null
    };

    class JsonValue {
        JsonType _curType; // 当前的json类型
        std::variant<std::unordered_map<std::string, JsonValue>, std::vector<JsonValue>, std::string, long long,
            long double, bool, std::nullptr_t>
        _curVal;

    public:
        template<typename T, typename = enableIfJson<T> >
        JsonValue(T &&val) {
            if constexpr (std::is_same_v<std::decay_t<T>, std::unordered_map<std::string, JsonValue> >) {
                _curVal.emplace<std::unordered_map<std::string, JsonValue> >(std::forward<T>(val));
                _curType = JsonType::Object;
            } else if constexpr (std::is_same_v<std::decay_t<T>, std::vector<JsonValue> >) {
                _curVal.emplace<std::vector<JsonValue> >(std::forward<T>(val));
                _curType = JsonType::Array;
            } else if constexpr (std::is_same_v<std::decay_t<T>, std::string> ||
                                 std::is_same_v<std::decay_t<T>, const char *>) {
                _curVal.emplace<std::string>(std::forward<T>(val));
                _curType = JsonType::String;
            } else if constexpr (std::is_same_v<std::decay_t<T>, bool>) {
                _curVal.emplace<bool>(std::forward<T>(val));
                _curType = JsonType::Bool;
            } else if constexpr (std::is_same_v<std::decay_t<T>, std::nullptr_t>) {
                _curVal.emplace<std::nullptr_t>(std::forward<T>(val));
                _curType = JsonType::Null;
            } else if constexpr (std::is_integral_v<T>) {
                _curVal.emplace<long long>(std::forward<T>(val));
                _curType = JsonType::Int;
            } else if constexpr (std::is_floating_point_v<T>) {
                _curVal.emplace<long double>(std::forward<T>(val));
                _curType = JsonType::Float;
            }
        }

        template<typename T, typename = enableIfJson<T> >
        JsonValue &operator=(T &&val) noexcept {
            if constexpr (std::is_same_v<std::decay_t<T>, std::unordered_map<std::string, JsonValue> >) {
                _curVal.emplace<std::unordered_map<std::string, JsonValue> >(std::forward<T>(val));
                _curType = JsonType::Object;
            } else if constexpr (std::is_same_v<std::decay_t<T>, std::vector<JsonValue> >) {
                _curVal.emplace<std::vector<JsonValue> >(std::forward<T>(val));
                _curType = JsonType::Array;
            } else if constexpr (std::is_same_v<std::decay_t<T>, std::string> ||
                                 std::is_same_v<std::decay_t<T>, const char *>) {
                _curVal.emplace<std::string>(std::forward<T>(val));
                _curType = JsonType::String;
            } else if constexpr (std::is_same_v<std::decay_t<T>, bool>) {
                _curVal.emplace<bool>(std::forward<T>(val));
                _curType = JsonType::Bool;
            } else if constexpr (std::is_same_v<std::decay_t<T>, std::nullptr_t>) {
                _curVal.emplace<std::nullptr_t>(std::forward<T>(val));
                _curType = JsonType::Null;
            } else if constexpr (std::is_integral_v<T>) {
                _curVal.emplace<long long>(std::forward<T>(val));
                _curType = JsonType::Int;
            } else if constexpr (std::is_floating_point_v<T>) {
                _curVal.emplace<long double>(std::forward<T>(val));
                _curType = JsonType::Float;
            }

            return *this;
        }

        [[nodiscard]] static JsonValue makeArr(const std::initializer_list<JsonValue> arr) noexcept {
            return {std::vector<JsonValue>{arr}};
        }

        [[nodiscard]] static JsonValue makeObj(
            const std::initializer_list<std::pair<std::string, JsonValue> > obj) noexcept {
            return {std::unordered_map<std::string, JsonValue>{obj.begin(), obj.end()}};
        }

        [[nodiscard]] JsonType getType() const noexcept {
            return _curType;
        }

        template<JsonType Type>
        auto getVal() const {
            if (Type != _curType) {
                const std::string funcInfo("in function getval()!");
                throw std::runtime_error(ERR_TYPE_MISMATCH + funcInfo);
            }

            if constexpr (Type == JsonType::Object)
                return std::get<std::unordered_map<std::string, JsonValue> >(_curVal);
            else if constexpr (Type == JsonType::Array)
                return std::get<std::vector<JsonValue> >(_curVal);
            else if constexpr (Type == JsonType::String)
                return std::get<std::string>(_curVal);
            else if constexpr (Type == JsonType::Bool)
                return std::get<bool>(_curVal);
            else if constexpr (Type == JsonType::Int)
                return std::get<long long>(_curVal);
            else if constexpr (Type == JsonType::Float)
                return std::get<long double>(_curVal);
            else if constexpr (Type == JsonType::Null)
                return std::get<std::nullptr_t>(_curVal);
        }

        friend std::ostream &operator<<(std::ostream &os, const JsonValue &val) {
            // json对象和数组底层分别是哈希表和vector，这里我们就不打印出来了
            const auto jsonObjectInfo = "Json Type: Json Object";
            const auto jsonArrayInfo = "Json Type: Json Array";

            switch (val._curType) {
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
    };
} // namespace simpleJson

#endif // JSON_TYPES_H
