#ifndef JSONPARSER_H
#define JSONPARSER_H

#include <stdint.h>
#include <fstream>
#include <stack>
#include <string>
#include "jsonTypes.h"

class JsonFile {
   public:
    JsonFile();
    ~JsonFile();

    void open_json(const char* fileName);  // may throw exception: runtime_error
    void close_json() noexcept;
    bool is_open() noexcept;

   private:
    std::string _fileName;
    std::fstream _file;

    std::string _jsonStr;  // the json data, raw str data from the .json file
    JsonType* _jsonData;   // the json data, maybe json object or json array

    // check the json syntax, if there is a syntax problem, throw the exception
    void _check_syntax();  // may throw exception: runtime_error

    void _load_data();  // may throw exception: runtime_error
};

#endif  // JSONPARSER_H