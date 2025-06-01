#include "jsonTypes.h"


/* ---JsonInt--- */
JsonInt::JsonInt(long long value) : _int(value) {}

bool JsonInt::operator==(const JsonInt &obj) const { return _int == obj._int; }
bool JsonInt::operator==(long long value) const { return _int == value; }

JsonInt::Type JsonInt::type() const { return Type::Int; }


/* ---JsonFlt--- */
JsonFlt::JsonFlt(double value) : _flt(value) {}

bool JsonFlt::operator==(const JsonFlt &obj) const { return _flt == obj._flt; }
bool JsonFlt::operator==(double value) const { return _flt == value; }

JsonFlt::Type JsonFlt::type() const { return Type::Float; }

/* ---JsonBool--- */
JsonBool::JsonBool(bool value) : _flag(value) {}

bool JsonBool::operator==(const JsonBool &obj) const { return _flag == obj._flag; }
bool JsonBool::operator==(bool value) const { return _flag == value; }

JsonBool::Type JsonBool::type() const { return Type::Bool; }

/* ---JsonStr--- */
JsonStr::JsonStr(const std::string &value) : _str(value) {}

const std::string &JsonStr::string() const { return _str; }

void JsonStr::push(const JsonStr &obj) { _str.append(obj._str); }
void JsonStr::push(const char *str) { _str.append(str); }

bool JsonStr::operator==(const JsonStr &obj) const { return _str == obj._str; }

bool JsonStr::operator==(const std::string &value) const { return _str == value; }

bool JsonStr::operator==(const char *value) const { return _str.c_str() == value; }

char &JsonStr::operator[](size_t pos) { return _str[pos]; }

JsonStr::Type JsonStr::type() const { return Type::String; }
size_t JsonStr::size() const { return _str.size(); }


/* ---JsonObj--- */
JsonObj::JsonObj() {}

JsonObj::Type JsonObj::type() const { return Type::Object; }
size_t JsonObj::size() const { return _dict.size(); }

JsonValue *JsonObj::operator[](const JsonStr &key) { return _dict[key.string()]; }
JsonValue *JsonObj::operator[](const std::string &key) { return _dict[key]; }
JsonValue *JsonObj::operator[](const char *key) { return _dict[key]; }

void JsonObj::push(const std::pair<std::string, JsonValue *> &pair) {}


/* ---JsonArr--- */
JsonArr::JsonArr() {}

JsonArr::Type JsonArr::type() const { return Type::Array; }
size_t JsonArr::size() const { return _arr.size(); }

JsonValue *JsonArr::operator[](size_t pos) { return _arr[pos]; }

/* ---JsonNull--- */
JsonNull::JsonNull() {}

JsonNull::Type JsonNull::type() const { return Type::Null; }
