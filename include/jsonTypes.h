#ifndef JSONTYPES_H
#define JSONTYPES_H

#include <string>
#include <unordered_map>
#include <vector>
#include "config.h"

class JsonType {
   public:
    JsonType(JsonTypeName type);
    virtual ~JsonType() = default;

    JsonTypeName type();

   private:
    JsonTypeName _type;
};

class JsonInt : public JsonType {
   public:
    JsonInt();

   private:
    long long _inte;
};

class JsonFlt : public JsonType {
   public:
    JsonFlt();

   private:
    double _flt;
};

class JsonStr : public JsonType {
   public:
    JsonStr();
    bool operator==(const JsonStr& obj);

   private:
    std::string _str;
};

class JsonBool : public JsonType {
   public:
    JsonBool();

   private:
    bool _flag;
};

class JsonObj : public JsonType {
   public:
    JsonObj();

   private:
    std::unordered_map<std::string, JsonType*> _dict;
};

class JsonArr : public JsonType {
   public:
    JsonArr();

   private:
    std::vector<JsonType*> _arr;
};

class JsonNull : public JsonType {
   public:
    JsonNull();

   private:
};

#endif  // JSONTYPES_H