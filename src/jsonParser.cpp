#include "jsonParser.h"
#include <algorithm>
#include <cctype>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include "config.h"
#include "jsonTypes.h"

JsonFile::JsonFile() : _jsonData(nullptr) {}

JsonFile::~JsonFile() {
    if (is_open()) {
        _file.close();
    }
}

void JsonFile::open_json(const char* fileName) {
    std::filesystem::path jsonPath(fileName);
    if (!std::filesystem::exists(jsonPath)) {
        throw std::runtime_error("json file does not exist!");
    }

    if (!std::filesystem::is_regular_file(jsonPath)) {
        throw std::runtime_error(
            "path should point to json file instaead of a directory!");
    }

    std::string extension = jsonPath.extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(),
                   ::tolower);
    if (extension != ".json") {
        throw std::runtime_error("the opened file is not json file!");
    }

    _fileName = jsonPath.filename().string();
    _file.open(jsonPath.string(), std::ios::in | std::ios::out);
    if (!_file.is_open()) {
        throw std::runtime_error("error opening json file!");
    }

    // load json data to string from the .json file
    _load_data();
}

void JsonFile::close_json() noexcept {
    if (is_open()) {
        _file.close();
    }
}

bool JsonFile::is_open() noexcept {
    return _file.is_open();
}

void JsonFile::_check_syntax() {
    if (_jsonStr.front() != '{' && _jsonStr.front() != '[') {
        throw std::runtime_error(
            "bad json structure! json data must be array or object");
    }

    // check whether the json file structure is right
    // std::stack<char> _struct;  // mark thr structure of the json file
    // for (size_t i = 0; i < _jsonStr.size(); ++i) {
    //     if (_jsonStr[i] == '{' || _jsonStr[i] == '[') {
    //         _struct.push(_jsonStr[i]);
    //     } else if (_jsonStr[i] == '}' || _jsonStr[i] == ']') {
    //         if ((_struct.top() == '{' && _jsonStr[i] == '}') ||
    //             (_struct.top() == '[' && _jsonStr[i] == ']')) {
    //             _struct.pop();
    //         } else {
    //             _struct.push(_jsonStr[i]);
    //         }
    //     }
    // }
    // if (!_struct.empty()) {
    //     throw std::runtime_error(
    //         "bad json structure! array or object not be closed");
    // }

    std::stack<JsonTypeName> curtype;
    JsonTypeName lastType;

    bool rColon = false;  // now on the right side of the :
    bool rComma = false;  // now on the right side of the ,

    for (size_t i = 0; i < _jsonStr.size(); ++i) {
        // mark the current json array or json object
        if (_jsonStr[i] == '{') {
            curtype.push(JsonTypeName::JsonObject);
        } else if (_jsonStr[i] == '[') {
            curtype.push(JsonTypeName::JsonArray);
        } else if (_jsonStr[i] == '}' &&
                   curtype.top() == JsonTypeName::JsonObject) {
            lastType = curtype.top();
            curtype.pop();
        } else if (_jsonStr[i] == ']' &&
                   curtype.top() == JsonTypeName::JsonArray) {
            lastType = curtype.top();
            curtype.pop();
        }

        // mark the json string
        if (_jsonStr[i] == '\"' && curtype.top() != JsonTypeName::JsonString) {
            curtype.push(JsonTypeName::JsonString);
        } else if (_jsonStr[i] == '\"' &&
                   curtype.top() == JsonTypeName::JsonString) {
            lastType = curtype.top();
            curtype.pop();
        }

        // mark the json int
        if (std::isdigit(_jsonStr[i])) {
            // if()
        }

        // mark the json float
        // ...

        if (std::isalpha(_jsonStr[i])) {
            if (_jsonStr[i] == 'e' && curtype.top() == JsonTypeName::JsonInt &&
                std::isdigit(_jsonStr[i + 1])) {
                // do nothing
            } else if (curtype.top() == JsonTypeName::JsonString) {
                // do nothing
            } else {
                throw std::runtime_error("json string must use \"\"");
            }
        }

        if (_jsonStr[i] == ':') {
            rColon = true;  // now we are on the right side of the colon(:)
        } else if (_jsonStr[i] == ',') {
            rColon = false;  // now we must on the left side of the colon(:)
            rComma = true;
        }
        if (_jsonStr[i] == '}' && rColon == false) {
            throw std::runtime_error(
                "the element of the json object must exist with key-value!");
        }
    }
    if (!curtype.empty()) {
        throw std::runtime_error(
            "bad json structure! array or object not closed");
    }
}

void JsonFile::_load_data() {
    // get the json data string
    char buffer[1024] = {0};
    while (!_file.eof()) {
        _file.getline(buffer, sizeof(buffer));
        _jsonStr += buffer;

        // std::string tmp(buffer);
        // size_t begin = tmp.find_first_not_of(" \t\n\r");
        // size_t end = tmp.find_last_not_of(" \t\n\r");
        // if (begin == std::string::npos) {  // meet the empty line
        //     continue;
        // }
        // _jsonStr += tmp.substr(begin, end + 1);
    }
    size_t begin = _jsonStr.find_first_not_of(" \t\n\r");
    _jsonStr.erase(0, begin);
    size_t end = _jsonStr.find_last_not_of(" \t\n\r");
    _jsonStr.erase(end + 1);

    std::cout << _jsonStr << std::endl;

    // check the json file syntax
    _check_syntax();

    // new a JsonType object to mark json object or json array
    if (_jsonStr.front() == '{') {
        _jsonData = new JsonObj;
    } else if (_jsonStr.front() == '[') {
        _jsonData = new JsonArr;
    }
}
