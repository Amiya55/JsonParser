#include "jsonTypes.h"

namespace simpleJson {
#if __cplusplus >= 201703L
    JsonValue::JsonValue()
        : _type(JsonType::Null)
        , _value(nullptr) {}

    JsonValue::JsonValue(int value)
        : _type(JsonType::Int)
        , _value(value) {}

    JsonValue::JsonValue(double value)
        : _type(JsonType::Float)
        , _value(value) {}

    JsonValue::JsonValue(bool value)
        : _type(JsonType::Bool)
        , _value(value) {}

    JsonValue::JsonValue(const std::string& value)
        : _type(JsonType::String)
        , _value(value) {}

    JsonValue::JsonValue(const std::vector<JsonValue> &value)
        : _type(JsonType::Array)
        , _value(value) {}

    JsonValue::JsonValue(const std::unordered_map<std::string, JsonValue> &value)
        : _type(JsonType::Object)
        , _value(value) {}

    JsonType JsonValue::getType() const {
        return _type;
    }

#else  // _cplusplus >= 201703L
    JsonValue::JsonData::JsonData() {
        // nothing to do
    }

    JsonValue::JsonData::~JsonData() {
        // nothing to do
    }

    JsonValue::JsonValue()
        : _type(JsonType::Null) {
        _data._null = nullptr;
    }

    JsonValue::JsonValue(long long value)
        : _type(JsonType::Int) {
        _data._int = value;
    }

    JsonValue::JsonValue(double value)
        : _type(JsonType::Float) {
        _data._float = value;
    }

    JsonValue::JsonValue(bool value)
        : _type(JsonType::Bool) {
        _data._bool = value;
    }

    JsonValue::JsonValue(const std::string &value)
        : _type(JsonType::String) {
        new(&_data._string) std::string(value);
    }

    JsonValue::JsonValue(const std::vector<JsonValue> &value)
        : _type(JsonType::Array) {
        new(&_data._array) std::vector<JsonValue>(value);
    }

    JsonValue::JsonValue(const std::unordered_map<std::string, JsonValue> &value)
        : _type(JsonType::Object) {
        new(&_data._object) std::unordered_map<std::string, JsonValue>(value);
    }

    JsonValue::JsonValue(const JsonValue &value) {
        _type = value._type;
        switch (value._type) {
            case JsonType::Null:
                // nothing to do
                break;
            case JsonType::Int:
                _data._int = value._data._int;
                break;
            case JsonType::Float:
                _data._float = value._data._float;
                break;
            case JsonType::Bool:
                _data._bool = value._data._bool;
                break;
            case JsonType::Object:
                new(&_data._object) std::unordered_map<std::string, JsonValue>(value._data._object);
                break;
            case JsonType::Array:
                new(&_data._array) std::vector<JsonValue>(value._data._array);
                break;
            case JsonType::String:
                new(&_data._string) std::string(value._data._string);
                break;
            default:
                ;
        }
    }

    JsonValue::~JsonValue() {
        switch (_type) {
            case JsonType::Object:
                _data._object.~unordered_map();
                break;
            case JsonType::Array:
                _data._array.~vector();
                break;
            case JsonType::String:
                _data._string.~basic_string();
                break;
            default:
                ;
        }
    }

    JsonValue &JsonValue::operator=(const JsonValue &value) {
        if (this == &value) {
            this->~JsonValue();
        }

        _type = value._type;
        switch (value._type) {
            case JsonType::Null:
                // nothing to do
                break;
            case JsonType::Int:
                _data._int = value._data._int;
                break;
            case JsonType::Float:
                _data._float = value._data._float;
                break;
            case JsonType::Bool:
                _data._bool = value._data._bool;
                break;
            case JsonType::String:
                new(&_data._string) std::string(value._data._string);
                break;
            case JsonType::Array:
                new(&_data._array) std::vector<JsonValue>(value._data._array);
                break;
            case JsonType::Object:
                new(&_data._object) std::unordered_map<std::string, JsonValue>(value._data._object);
                break;
            default:
                ;
        }

        return *this;
    }


    JsonType JsonValue::getType() const {
        return _type;
    }
#endif  // _cplusplus >= 201703L
}
