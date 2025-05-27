#include "jsonTypes.h"
#include "config.h"

JsonType::JsonType(JsonTypeName type) : _type(type) {}

JsonTypeName JsonType::type() {
    return _type;
}

JsonStr::JsonStr() : JsonType(JsonTypeName::JsonString) {}

bool JsonStr::operator==(const JsonStr& obj) {
    return _str == obj._str;
}

JsonInt::JsonInt() : JsonType(JsonTypeName::JsonInt) {}

JsonFlt::JsonFlt() : JsonType(JsonTypeName::JsonFloat) {}

JsonBool::JsonBool() : JsonType(JsonTypeName::JsonBool) {}

JsonNull::JsonNull() : JsonType(JsonTypeName::JsonNull) {}

JsonObj::JsonObj() : JsonType(JsonTypeName::JsonObject) {}

JsonArr::JsonArr() : JsonType(JsonTypeName::JsonArray) {}