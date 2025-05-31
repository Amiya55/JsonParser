#include "jsonParser.h"
#include <algorithm>
#include <cctype>
#include <cstddef>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stack>
#include <stdexcept>
#include <string>
#include "config.h"
#include "jsonTypes.h"


/* --- JsonSyntaxChecker --- */
void JsonSyntaxChecker::_check_key(const std::string& key) {
    bool le = false;
    bool ri = false;
    for (int i = 0; i < key.size(); ++i) {
        if (le == false) {
            if (key[i] == '\"')
                le = true;
            else if (key[i] != ' ')
                throw std::runtime_error(
                    "Property keys must be doublequoted: " + key);
        } else if (le == true && ri == false) {
            if (key[i] == '\"' && key[i - 1] != '\\')
                ri = true;
        } else if (le == true && ri == true) {
            if (key[i] != ' ')
                throw std::runtime_error("Unexcpected property key: " + key);
        }
    }
    if (le == false || ri == false)
        throw std::runtime_error("Unexpected end of string: " + key);
}

void JsonSyntaxChecker::_check_value(const std::string& value) {
    if (value.find_first_not_of(" \t\n\r") == std::string::npos)
        throw std::runtime_error("Value expected");

    std::stack<JsonTypeName> type;
    int countE = 0;
    for (int i = 0; i < value.size(); ++i) {
        if (type.empty()) {
            if (value[i] == '\"') {
                // 如果是"并且栈为空，则认为进入了字符串
                type.push(JsonTypeName::JsonString);
            } else if (value[i] == '{' || value[i] == '[') {
                // 还要判断一下会不会出现{"C++"}1234这种情况
                size_t last = value.find_last_not_of(" \t\n\r");
                if (value[last] == '}' || value[last] == ']')
                    return;
                else
                    throw std::runtime_error(
                        "Unexpecte value, maybe you fogot close your {} or "
                        "[]: " +
                        value);
            } else if (isdigit(value[i])) {
                type.push(JsonTypeName::JsonInt);
            } else if (value[i] != '\"' && value[i] != ' ') {
                // 如果在引号之外出现了字符，那么我们直接判断这个字符串是不是null，不是，则认为value值非法
                size_t end = value.find_last_not_of(" \t\n\r");
                if (value.substr(i, end + 1) == "null")
                    return;
                else
                    throw std::runtime_error("Unexpected value: " + value);
            }
        } else if (type.top() == JsonTypeName::JsonString) {
            if (value[i] == '\"' && value[i - 1] != '\\') {
                // 字符为"并且栈中标记为STR，并且前一位不是\，就认为字符串结束，value值结束
                type.pop();
                if (value.substr(i + 1).find_first_not_of(" \t\n\r") !=
                    std::string::npos)
                    // 假设value值结束还存在非空格字符，则认为value值非法
                    throw std::runtime_error("Expected comma: " + value);
            }
        } else if ((type.top() == JsonTypeName::JsonInt ||
                    type.top() == JsonTypeName::JsonFloat)) {
            if (value[i] == ' ') {
                // 如果当前的类型为float和int，如果当前字符为空格，则认为value值已经结束，可以收尾了
                type.pop();
                if (value.substr(i + 1).find_first_not_of(" \t\n\r") !=
                    std::string::npos)
                    // 如果value值结束了还有字符，那么肯定是因为原对象中缺了逗号，当前值非法
                    throw std::runtime_error("Expected comma: " + value);
            } else if (value[i] == 'e') {
                // 如果当前的类型为float或者int，并且当前字符不是数字，不是字母e，或者e的数量大于一，都认为value值非法
                ++countE;
                if (countE > 1 || i == value.size() - 1 ||
                    !isdigit(value[i + 1]))
                    throw std::runtime_error("Unexpected num: " + value);
            } else if (value[i] == '.' && countE == 0) {
                // e的后面不能有小数点
                if (type.top() == JsonTypeName::JsonInt) {
                    if (i == value.size() - 1 || !isdigit(value[i + 1])) {
                        // 如果小数点后面没有字符或者字符不是数字，直接返回false
                        throw std::runtime_error("Unexpected end of num: " +
                                                 value);
                    } else {
                        type.pop();
                        type.push(JsonTypeName::JsonFloat);
                    }
                } else {
                    // 如果已经是float类型，这时候再出现一次小数点，那么认为当前value非法
                    throw std::runtime_error("Unexpected num: " + value);
                }
            } else if (!isdigit(value[i])) {
                // 除了以上特殊字符情况，当前的字符不是数字，认为value值非法
                throw std::runtime_error("Unexpected num: " + value);
            }
        }
    }

    // 如果最后出循环，栈里面还有元素，并且元素还是STR，意味着字符串的引号未关闭，value值非法
    if (!type.empty() && type.top() == JsonTypeName::JsonString)
        throw std::runtime_error("Unexpected end of string: " + value);
}

void JsonSyntaxChecker::syntax_check(const std::string& jsonStr,
                                     JsonTypeName containerType) {
    std::string value;
    std::string
        subContainer; // 当前字符串中存在的子容器: {}或者[]，用作递归调用参数

    std::stack<JsonTypeName> types; // 当前字符所在的数据结构: "", {}, []
    std::vector<std::string> values; // obj字符串以逗号(,)分割开的子字符串

    size_t countComma = 0;
    bool inContainer = false;

    for (int i = 1; i < jsonStr.size() - 1; ++i) {
        if (jsonStr[i] == '\"') {
            if (types.empty() || types.top() != JsonTypeName::JsonString)
                types.push(JsonTypeName::JsonString);
            else if (types.top() == JsonTypeName::JsonString &&
                     jsonStr[i - 1] != '\\')
                types.pop();
        }

        if (jsonStr[i] == '{' || jsonStr[i] == '[') {
            if (types.empty() || types.top() != JsonTypeName::JsonString) {
                jsonStr[i] == '{'
                    ? types.push(JsonTypeName::JsonObject)
                    : types.push(JsonTypeName::JsonArray);
                inContainer = true;
            }
        } else if (jsonStr[i] == '}' || jsonStr[i] == ']') {
            if (!types.empty() && types.top() != JsonTypeName::JsonString) {
                const JsonTypeName lastType = types.top();
                types.pop();
                // 当栈为空，说明我们最初遇到的容器结束了，可以递归调用，传入这个容器字符串，进一步检查
                if (types.empty()) {
                    subContainer.push_back(jsonStr[i]);
                    lastType == JsonTypeName::JsonObject
                        ? syntax_check(subContainer, JsonTypeName::JsonObject)
                        : syntax_check(subContainer, JsonTypeName::JsonArray);
                    inContainer = false;
                    subContainer.clear();
                }
            }
        }

        if (jsonStr[i] == ',' && types.empty()) {
            ++countComma; // 只有当types栈为空时才加加这个值，否则可能是字符串或者容器内的逗号
            if (!value.empty() &&
                value.find_first_not_of(" \t\n\r") != std::string::npos)
                values.push_back(value);
            value.clear();
        } else {
            if (inContainer)
                subContainer.push_back(jsonStr[i]);
            value.push_back(jsonStr[i]);
        }

        if (i == jsonStr.size() - 2) {
            // 遇到json字符串结尾，push最后的数据到数组中
            if (value.find_first_not_of(" \t\n\r") != std::string::npos)
                values.push_back(value);

            // 最后检查types数组是否为空，如果不为空，那么说明""，{}或者[]存在没有关闭的情况
            if (!types.empty())
                throw std::runtime_error(
                    "Syntax Error, string or container does not closed: " +
                    jsonStr);
        }
    }

    if (!values.empty() && countComma >= values.size()) {
        throw std::runtime_error("unexpected comma: " + jsonStr);
    }

    // 检查语法
    for (int i = 0; i < values.size(); ++i) {
        if (containerType == JsonTypeName::JsonObject) {
            size_t sep = values[i].find_first_of(":");
            if (sep == std::string::npos) {
                throw std::runtime_error(
                    "json object expected key/value module: " + values[i]);
            }

            try {
                _check_key(values[i].substr(0, sep));
                _check_value(values[i].substr(sep + 1));
            } catch (const std::exception& e) {
                std::string errMsg(e.what());
                throw std::runtime_error(errMsg + " --> \n" + jsonStr);
            }
        } else if (containerType == JsonTypeName::JsonArray) {
            try {
                _check_value(values[i]);
            } catch (const std::exception& e) {
                std::string errMsg(e.what());
                throw std::runtime_error(errMsg + " --> \n" + jsonStr);
            }
        }
    }
}


/* --- JsonFile --- */
JsonFile::JsonFile() : _jsonData(nullptr) {}

JsonFile::~JsonFile() {
    if (is_open()) {
        _file.close();
    }
}

void JsonFile::open_json(const char* fileName) {
    // if there is a already be opened, close the file and open a new one
    if (is_open()) {
        close_json();
    }

    // check the json file exists or not
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

    // get file name and open the file
    _fileName = jsonPath.filename().string();
    _file.open(jsonPath.string(), std::ios::in | std::ios::out);
    if (!_file.is_open()) {
        throw std::runtime_error("error opening json file!");
    }

    // get the json data string from json file
    std::string buffer;
    while (std::getline(_file, buffer)) {
        _jsonStr += buffer;
    }
    // get rid of the space before the json string or behind the json string
    size_t begin = _jsonStr.find_first_not_of(" \t\n\r");
    _jsonStr.erase(0, begin);
    size_t end = _jsonStr.find_last_not_of(" \t\n\r");
    _jsonStr.erase(end + 1);

    // new a JsonType object to mark json object or json array
    if (_jsonStr.front() == '{') {
        // check the json file syntax
        JsonSyntaxChecker().syntax_check(_jsonStr, JsonTypeName::JsonObject);
        _jsonData = new JsonObj;
    } else if (_jsonStr.front() == '[') {
        JsonSyntaxChecker().syntax_check(_jsonStr, JsonTypeName::JsonArray);
        _jsonData = new JsonArr;
    }

    // parse json str into our json classes
    _parse_data();
}

void JsonFile::close_json() noexcept {
    if (is_open()) {
        _file.close();
    }
}

bool JsonFile::is_open() noexcept {
    return _file.is_open();
}

void JsonFile::_parse_data() noexcept {}