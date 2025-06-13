#include "jsonTypes.h"
#include <stdexcept>

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

    JsonValue::JsonValue(std::nullptr_t val)
        : _type(JsonType::Null) {
        _data._null = val;
    }

    JsonValue::JsonValue(int value)
        : _type(JsonType::Int) {
        _data._int = value;
    }

    JsonValue::JsonValue(long value)
        : _type(JsonType::Int) {
        _data._int = value;
    }

    JsonValue::JsonValue(long long value)
        : _type(JsonType::Int) {
        _data._int = value;
    }

    JsonValue::JsonValue(float value)
        : _type(JsonType::Float) {
        _data._float = value;
    }

    JsonValue::JsonValue(double value)
        : _type(JsonType::Float) {
        _data._float = value;
    }

    JsonValue::JsonValue(bool value)
        : _type(JsonType::Bool) {
        _data._bool = value;
    }

    JsonValue::JsonValue(std::string &value)
        : _type(JsonType::String) {
        new(&_data._string) std::string(value);
    }

    JsonValue::JsonValue(std::string &&value)
        : _type(JsonType::String) {
        new(&_data._string) std::string(std::move(value));
    }

    JsonValue::JsonValue(const char *value)
        : _type(JsonType::String) {
        new(&_data._string) std::string(value);
    }

    JsonValue::JsonValue(std::vector<JsonValue> &value)
        : _type(JsonType::Array) {
        new(&_data._array) std::vector<JsonValue>(value);
    }

    JsonValue::JsonValue(std::vector<JsonValue> &&value)
        : _type(JsonType::Array) {
        new(&_data._array) std::vector<JsonValue>(std::move(value));
    }

    JsonValue::JsonValue(std::unordered_map<std::string, JsonValue> &value)
        : _type(JsonType::Object) {
        new(&_data._object) std::unordered_map<std::string, JsonValue>(value);
    }

    JsonValue::JsonValue(std::unordered_map<std::string, JsonValue> &&value)
        : _type(JsonType::Object) {
        new(&_data._object) std::unordered_map<std::string, JsonValue>(std::move(value));
    }

    JsonValue::JsonValue(const std::initializer_list<JsonValue> &value)
        : _type(JsonType::Array) {
        new(&_data._array) std::vector<JsonValue>(value);
    }

    JsonValue::JsonValue(const JsonValue &value) noexcept {
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

    void JsonValue::_destroy() noexcept {
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
        _type = JsonType::Null;
    }


    JsonValue::~JsonValue() {
        _destroy();
    }

    JsonValue &JsonValue::operator=(const JsonValue &value) noexcept {
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


    JsonType JsonValue::getType() const noexcept {
        return _type;
    }

    long long &JsonValue::getInt() {
        if (_type != JsonType::Int) {
            throw std::runtime_error("json type is not int");
        }
        return _data._int;
    }

    double &JsonValue::getFloat() {
        if (_type != JsonType::Float) {
            throw std::runtime_error("json type is not float");
        }
        return _data._float;
    }

    bool &JsonValue::getBool() {
        if (_type != JsonType::Bool) {
            throw std::runtime_error("json type is not bool");
        }
        return _data._bool;
    }

    std::string &JsonValue::getString() {
        if (_type != JsonType::String) {
            throw std::runtime_error("json type is not string");
        }
        return _data._string;
    }

    std::vector<JsonValue> &JsonValue::getArray() {
        if (_type != JsonType::Array) {
            throw std::runtime_error("json type is not array");
        }
        return _data._array;
    }

    std::unordered_map<std::string, JsonValue> &JsonValue::getObject() {
        if (_type != JsonType::Object) {
            throw std::runtime_error("json type is not object");
        }
        return _data._object;
    }

    JsonValue &JsonValue::operator=(std::nullptr_t val) {
        _destroy();

        _data._null = val;
        _type = JsonType::Null;

        return *this;
    }

    JsonValue &JsonValue::operator=(int val) {
        _destroy();

        _data._int = val;
        _type = JsonType::Int;

        return *this;
    }


    JsonValue &JsonValue::operator=(long val) noexcept {
        _destroy();

        _data._int = val;
        _type = JsonType::Int;

        return *this;
    }


    JsonValue &JsonValue::operator=(long long val) noexcept {
        _destroy();

        _data._int = val;
        _type = JsonType::Int;

        return *this;
    }

    JsonValue &JsonValue::operator=(float val) noexcept {
        _destroy();

        _data._float = val;
        _type = JsonType::Float;

        return *this;
    }

    JsonValue &JsonValue::operator=(double val) noexcept {
        _destroy();

        _data._float = val;
        _type = JsonType::Float;

        return *this;
    }

    JsonValue &JsonValue::operator=(bool val) noexcept {
        _destroy();

        _data._bool = val;
        _type = JsonType::Bool;

        return *this;
    }

    JsonValue &JsonValue::operator=(std::string &val) noexcept {
        _destroy();

        new(&_data._string) std::string(val);
        _type = JsonType::String;

        return *this;
    }

    JsonValue &JsonValue::operator=(std::string &&val) noexcept {
        _destroy();

        new(&_data._string) std::string(std::move(val));
        _type = JsonType::String;

        return *this;
    }

    JsonValue &JsonValue::operator=(const char *val) noexcept {
        _destroy();

        new(&_data._string) std::string(val);
        _type = JsonType::String;

        return *this;
    }

    JsonValue &JsonValue::operator=(std::vector<JsonValue> &val) noexcept {
        _destroy();

        new(&_data._array) std::vector<JsonValue>(val);
        _type = JsonType::Array;

        return *this;
    }

    JsonValue &JsonValue::operator=(std::vector<JsonValue> &&val) noexcept {
        _destroy();

        new(&_data._array) std::vector<JsonValue>(std::move(val));
        _type = JsonType::Array;

        return *this;
    }

    JsonValue &JsonValue::operator=(std::unordered_map<std::string, JsonValue> &val) noexcept {
        _destroy();

        new(&_data._object) std::unordered_map<std::string, JsonValue>(val);
        _type = JsonType::Object;

        return *this;
    }

    JsonValue &JsonValue::operator=(std::unordered_map<std::string, JsonValue> &&val) noexcept {
        _destroy();

        new(&_data._object) std::unordered_map<std::string, JsonValue>(std::move(val));
        _type = JsonType::Object;

        return *this;
    }

    void JsonValue::arrPush(JsonValue &val) {
        if (_type != JsonType::Array) {
            throw std::logic_error("json type is not array");
        }

        _data._array.push_back(val);
    }

    void JsonValue::arrPush(JsonValue &&val) {
        if (_type != JsonType::Array) {
            throw std::logic_error("json type is not array");
        }

        _data._array.push_back(std::move(val));
    }

    JsonValue &JsonValue::objPush(const std::pair<std::string, JsonValue> &val) {
        if (_type != JsonType::Object) {
            throw std::logic_error("json type is not object");
        }

        std::string jsonKey("\"" + val.first + "\"");
        if (_data._object.find(jsonKey) != _data._object.end()) {
            return _data._object[jsonKey];
        }

        _data._object[jsonKey] = val.second;
        return _data._object[jsonKey];
    }


    JsonValue &JsonValue::operator[](size_t index) {
        if (_type != JsonType::Array) {
            throw std::logic_error("json type is not array");
        }

        return _data._array[index];
    }

    JsonValue &JsonValue::operator[](const std::string &key){
        if (_type != JsonType::Object) {
            throw std::logic_error("json type is not object");
        }

        return objPush(std::make_pair<const std::string&, JsonValue>(key, nullptr));
    }

#endif  // _cplusplus >= 201703L
}
