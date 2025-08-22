#ifndef JSON_TYPES_H
#define JSON_TYPES_H

#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

#include "config.h"

namespace simple_json
{
// 明确json的数据类型有哪些
enum class JsonType : uint8_t
{
    Object,
    Array,
    String,
    Int,
    Float,
    Bool,
    Null
};
std::ostream &operator<<(std::ostream &os, const JsonType &type);

class JsonValue
{
    JsonType cur_type_; // 当前的json类型
    std::variant<std::unordered_map<std::string, JsonValue>, std::vector<JsonValue>, std::string, long long,
                 long double, bool, std::nullptr_t>
        cur_val_;

  public:
    template <typename T, typename = enableIfJson<T>> JsonValue(T &&val)
    {
        if constexpr (std::is_same_v<std::decay_t<T>, std::unordered_map<std::string, JsonValue>>)
        {
            cur_val_.emplace<std::unordered_map<std::string, JsonValue>>(std::forward<T>(val));
            cur_type_ = JsonType::Object;
        }
        else if constexpr (std::is_same_v<std::decay_t<T>, std::vector<JsonValue>>)
        {
            cur_val_.emplace<std::vector<JsonValue>>(std::forward<T>(val));
            cur_type_ = JsonType::Array;
        }
        else if constexpr (std::is_same_v<std::decay_t<T>, std::string> ||
                           std::is_same_v<std::decay_t<T>, const char *>)
        {
            cur_val_.emplace<std::string>(std::forward<T>(val));
            cur_type_ = JsonType::String;
        }
        else if constexpr (std::is_same_v<std::decay_t<T>, bool>)
        {
            cur_val_.emplace<bool>(std::forward<T>(val));
            cur_type_ = JsonType::Bool;
        }
        else if constexpr (std::is_same_v<std::decay_t<T>, std::nullptr_t>)
        {
            cur_val_.emplace<std::nullptr_t>(std::forward<T>(val));
            cur_type_ = JsonType::Null;
        }
        else if constexpr (std::is_integral_v<T>)
        {
            cur_val_.emplace<long long>(std::forward<T>(val));
            cur_type_ = JsonType::Int;
        }
        else if constexpr (std::is_floating_point_v<T>)
        {
            cur_val_.emplace<long double>(std::forward<T>(val));
            cur_type_ = JsonType::Float;
        }
    }

    template <typename T, typename = enableIfJson<T>> JsonValue &operator=(T &&val) noexcept
    {
        if constexpr (std::is_same_v<std::decay_t<T>, std::unordered_map<std::string, JsonValue>>)
        {
            cur_val_.emplace<std::unordered_map<std::string, JsonValue>>(std::forward<T>(val));
            cur_type_ = JsonType::Object;
        }
        else if constexpr (std::is_same_v<std::decay_t<T>, std::vector<JsonValue>>)
        {
            cur_val_.emplace<std::vector<JsonValue>>(std::forward<T>(val));
            cur_type_ = JsonType::Array;
        }
        else if constexpr (std::is_same_v<std::decay_t<T>, std::string> ||
                           std::is_same_v<std::decay_t<T>, const char *>)
        {
            cur_val_.emplace<std::string>(std::forward<T>(val));
            cur_type_ = JsonType::String;
        }
        else if constexpr (std::is_same_v<std::decay_t<T>, bool>)
        {
            cur_val_.emplace<bool>(std::forward<T>(val));
            cur_type_ = JsonType::Bool;
        }
        else if constexpr (std::is_same_v<std::decay_t<T>, std::nullptr_t>)
        {
            cur_val_.emplace<std::nullptr_t>(std::forward<T>(val));
            cur_type_ = JsonType::Null;
        }
        else if constexpr (std::is_integral_v<T>)
        {
            cur_val_.emplace<long long>(std::forward<T>(val));
            cur_type_ = JsonType::Int;
        }
        else if constexpr (std::is_floating_point_v<T>)
        {
            cur_val_.emplace<long double>(std::forward<T>(val));
            cur_type_ = JsonType::Float;
        }

        return *this;
    }

    JsonValue() : cur_type_(JsonType::Null), cur_val_(nullptr)
    {
    }
    ~JsonValue() = default;

    JsonValue(const JsonValue &other) noexcept = default;
    JsonValue(JsonValue &&other) noexcept = default;
    JsonValue &operator=(const JsonValue &other) noexcept = default;
    JsonValue &operator=(JsonValue &&other) noexcept = default;

    /**
     * @brief Creates a JSON object from a C++ array.
     *
     * @param arr C++ array with an initializer list.
     * @return JsonValue contains a C++ array
     */
    [[nodiscard]] static JsonValue MakeArr(const std::initializer_list<JsonValue> arr) noexcept
    {
        return {std::vector<JsonValue>{arr}};
    }

    /**
     * @brief Creates a JSON object from a C++ hash map.
     *
     * @param obj C++ hash map with an initializer list.
     * @return JsonValue contains a C++ hash map
     */
    [[nodiscard]] static JsonValue MakeObj(const std::initializer_list<std::pair<std::string, JsonValue>> obj) noexcept
    {
        return {std::unordered_map<std::string, JsonValue>{obj.begin(), obj.end()}};
    }

    /**
     * @brief Gets the type stored in the current JsonValue object.
     *
     * @return  Specific JsonType
     */
    [[nodiscard]] JsonType GetType() const noexcept
    {
        return cur_type_;
    }

    /**
     * @brief Get current C++ value in JsonValue object
     *
     * @tparam Type - The specified C++ type to be retrieved.
     * @return auto - The specified C++ const reference value from the JsonValue object.
     */
    template <JsonType Type> const auto& GetVal() const
    {
        if (Type != cur_type_)
        {
            const std::string func_info("in function getval()!");
            throw std::runtime_error(ERR_TYPE_MISMATCH + func_info);
        }

        if constexpr (Type == JsonType::Object)
        {
            return std::get<std::unordered_map<std::string, JsonValue>>(cur_val_);
        }
        else if constexpr (Type == JsonType::Array)
        {
            return std::get<std::vector<JsonValue>>(cur_val_);
        }
        else if constexpr (Type == JsonType::String)
        {
            return std::get<std::string>(cur_val_);
        }
        else if constexpr (Type == JsonType::Bool)
        {
            return std::get<bool>(cur_val_);
        }
        else if constexpr (Type == JsonType::Int)
        {
            return std::get<long long>(cur_val_);
        }
        else if constexpr (Type == JsonType::Float)
        {
            return std::get<long double>(cur_val_);
        }
        else if constexpr (Type == JsonType::Null)
        {
            return std::get<std::nullptr_t>(cur_val_);
        }
    }

    /**
     * @brief Get current C++ value in JsonValue object
     *
     * @tparam Type - The specified C++ type to be retrieved.
     * @return auto - The specified C++ reference value from the JsonValue object.
     */
    template <JsonType Type> auto &GetVal()
    {
        if (Type != cur_type_)
        {
            const std::string func_info("in function getval()!");
            throw std::runtime_error(ERR_TYPE_MISMATCH + func_info);
        }

        if constexpr (Type == JsonType::Object)
        {
            return std::get<std::unordered_map<std::string, JsonValue>>(cur_val_);
        }
        else if constexpr (Type == JsonType::Array)
        {
            return std::get<std::vector<JsonValue>>(cur_val_);
        }
        else if constexpr (Type == JsonType::String)
        {
            return std::get<std::string>(cur_val_);
        }
        else if constexpr (Type == JsonType::Bool)
        {
            return std::get<bool>(cur_val_);
        }
        else if constexpr (Type == JsonType::Int)
        {
            return std::get<long long>(cur_val_);
        }
        else if constexpr (Type == JsonType::Float)
        {
            return std::get<long double>(cur_val_);
        }
        else if constexpr (Type == JsonType::Null)
        {
            return std::get<std::nullptr_t>(cur_val_);
        }
    }

    /**
     * @brief A friend function used to print the corresponding JsonValue type with std::cout.
     *
     * @param os ostream
     * @param val The specific JsonValue object
     * @return std::ostream&
     */
    friend std::ostream &operator<<(std::ostream &os, const JsonValue &val);
};
} // namespace simple_json

#endif // JSON_TYPES_H
