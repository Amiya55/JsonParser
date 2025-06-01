#ifndef JSONPARSER_H
#define JSONPARSER_H

#include <fstream>
#include <string>
#include "jsonTypes.h"

class JsonSyntaxChecker {
public:
    JsonSyntaxChecker() = default;
    ~JsonSyntaxChecker() = default;

    void syntax_check(const std::string &jsonStr,
                      JsonValue::Type containerType); // may throw exception: runtime_error

private:
    // check the key in json object
    void _check_key(const std::string &key); // may throw exception: runtime_error
    // check the value in json object, also it can check the element in json array
    void _check_value(const std::string &value); // may throw exception: runtime_error
};

class JsonFile {
public:
    JsonFile();
    ~JsonFile();

    void open_json(const char *fileName); // may throw exception: runtime_error
    void close_json() noexcept;
    bool is_open() noexcept;

private:
    std::string _fileName;
    std::fstream _file;

    std::string _jsonStr; // the json data, raw str data from the .json file
    JsonValue *_jsonData; // the json data, maybe json object or json array

    // transform the json str into our json classes and get ready to operate
    void _parse_data(JsonValue *mountPoint, std::string &strs) noexcept;
};

#endif // JSONPARSER_H
