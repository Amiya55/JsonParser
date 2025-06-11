#ifndef JsonValueS_H
#define JsonValueS_H

#include <string>
#include <unordered_map>
#include <vector>
#include <cstddef>

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
        explicit JsonValue();

        explicit JsonValue(int value);

        explicit JsonValue(long value);

        explicit JsonValue(long long value);

        explicit JsonValue(float value);

        explicit JsonValue(double value);

        explicit JsonValue(bool value);

        explicit JsonValue(std::string &value);

        explicit JsonValue(std::string &&value);

        explicit JsonValue(const char *value);

        explicit JsonValue(std::vector<JsonValue> &value);

        explicit JsonValue(std::vector<JsonValue> &&value);

        explicit JsonValue(std::unordered_map<std::string, JsonValue> &value);

        explicit JsonValue(std::unordered_map<std::string, JsonValue> &&value);


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

    private:
        void _destroy() noexcept;
    };
#endif  // _cplusplus >= 201703L
}

#endif // JsonValueS_H
