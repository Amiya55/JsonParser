#ifndef JsonValueS_H
#define JsonValueS_H

#include <cstddef>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

class JsonValue {
public:
    enum class Type { Object, Array, String, Int, Float, Bool, Null };

    virtual ~JsonValue() = default;
    virtual Type type() const = 0;
};


class JsonInt : public JsonValue {
public:
    explicit JsonInt(long long value);

    bool operator==(const JsonInt &obj) const;
    bool operator==(long long value) const;

    virtual Type type() const;

private:
    long long _int;
};


class JsonFlt : public JsonValue {
public:
    explicit JsonFlt(double value);

    bool operator==(const JsonFlt &obj) const;
    bool operator==(double value) const;

    virtual Type type() const;

private:
    double _flt;
};


class JsonBool : public JsonValue {
public:
    explicit JsonBool(bool value);

    bool operator==(const JsonBool &obj) const;
    bool operator==(bool value) const;

    virtual Type type() const;

private:
    bool _flag;
};


class JsonStr : public JsonValue {
public:
    explicit JsonStr(const std::string &value = "");

    const std::string &string() const;

    void push(const JsonStr &obj);
    void push(const char *str);

    bool operator==(const JsonStr &obj) const;
    bool operator==(const std::string &value) const;
    bool operator==(const char *value) const;

    char &operator[](size_t pos);

    virtual size_t size() const;
    virtual Type type() const;

private:
    std::string _str;
};


class JsonObj : public JsonValue {
public:
    explicit JsonObj();

    void push(const std::pair<std::string, JsonValue *> &pair);

    JsonValue *operator[](const JsonStr &key);
    JsonValue *operator[](const std::string &key);
    JsonValue *operator[](const char *key);

    virtual size_t size() const;
    virtual Type type() const;

private:
    std::unordered_map<std::string, JsonValue *> _dict;
};


class JsonArr : public JsonValue {
public:
    explicit JsonArr();

    void push(const JsonValue &obj);

    JsonValue *operator[](size_t pos);

    virtual size_t size() const;
    virtual Type type() const;

private:
    std::vector<JsonValue *> _arr;
};


class JsonNull : public JsonValue {
public:
    explicit JsonNull();

    virtual Type type() const;

private:
};

#endif // JsonValueS_H
