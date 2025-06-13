#ifndef JsonValueS_H
#define JsonValueS_H

#include <string>
#include <unordered_map>
#include <vector>

#include "config.h"

#if __cplusplus >= 201703L
#include <variant>
#endif  // __cplusplus >= 201703L

namespace simpleJson {
    enum class JsonType {
        Object, Array, String,
        Int, Float, Bool,
        Null
    };

#if __cplusplus >= 201703L
    class JsonValue {
        JsonType _type;
        std::variant<std::string, std::vector<JsonValue>,
            std::unordered_map<std::string, JsonValue>,
            int, double, bool, std::nullptr_t> _value;

    public:
        explicit JsonValue();

        explicit JsonValue(int value);

        explicit JsonValue(double value);

        explicit JsonValue(bool value);

        explicit JsonValue(const std::string &value);

        explicit JsonValue(const std::vector<JsonValue> &value);

        explicit JsonValue(const std::unordered_map<std::string, JsonValue> &value);

        [[nodiscard]] JsonType getType() const;

    };

#else  // _cplusplus >= 201703L
    class JsonValue {
        JsonType _type;

        union JsonData {
            std::unordered_map<std::string, JsonValue> _object;
            std::vector<JsonValue> _array;
            std::string _string;
            long long _int{};
            double _float;
            bool _bool;
            std::nullptr_t _null;

            JsonData();

            ~JsonData();
        } _data;

    public:
        /* 为了支持{"hello world", 10, false, nullptr}这样的隐式类型转换，我们不加explicit */
        JsonValue(std::nullptr_t value = nullptr);

        JsonValue(int value);

        JsonValue(long value);

        JsonValue(long long value);

        JsonValue(float value);

        JsonValue(double value);

        JsonValue(bool value);

        JsonValue(std::string &value);

        JsonValue(std::string &&value);

        JsonValue(const char *value);

        JsonValue(std::vector<JsonValue> &value);

        JsonValue(std::vector<JsonValue> &&value);

        JsonValue(std::unordered_map<std::string, JsonValue> &value);

        JsonValue(std::unordered_map<std::string, JsonValue> &&value);

        JsonValue(const std::initializer_list<JsonValue> &value);

        ~JsonValue();

        JsonValue(const JsonValue &value) noexcept;

        JsonValue &operator=(const JsonValue &value) noexcept;

        JsonType getType() const noexcept;

        long long &getInt();

        double &getFloat();

        bool &getBool();

        std::string &getString();

        std::vector<JsonValue> &getArray();

        std::unordered_map<std::string, JsonValue> &getObject();

        JsonValue &operator=(std::nullptr_t val);

        JsonValue &operator=(int val);

        JsonValue &operator=(long val) noexcept;

        JsonValue &operator=(long long val) noexcept;

        JsonValue &operator=(float val) noexcept;

        JsonValue &operator=(double val) noexcept;

        JsonValue &operator=(bool val) noexcept;

        JsonValue &operator=(std::string &val) noexcept;

        JsonValue &operator=(std::string &&val) noexcept;

        JsonValue &operator=(const char *val) noexcept;

        JsonValue &operator=(std::vector<JsonValue> &val) noexcept;

        JsonValue &operator=(std::vector<JsonValue> &&val) noexcept;

        JsonValue &operator=(std::unordered_map<std::string, JsonValue> &val) noexcept;

        JsonValue &operator=(std::unordered_map<std::string, JsonValue> &&val) noexcept;

        void arrPush(JsonValue &val);

        void arrPush(JsonValue &&val);

        JsonValue &objPush(const std::pair<std::string, JsonValue> &val);

        JsonValue &operator[](size_t index);

        JsonValue &operator[](const std::string &key);

    private:
        void _destroy() noexcept;
    };
#endif  // _cplusplus >= 201703L
}

#endif // JsonValueS_H
