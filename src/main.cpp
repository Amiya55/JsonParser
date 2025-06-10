#include <exception>
#include <iostream>
#include <fstream>
#include "iniParser.h"
#include "jsonParser.h"
#include "jsonTypes.h"
#include "utilities.h"

// #define TEST

std::string load_json() {
    std::fstream fs;
    fs.open("data.json", std::ios::in);
    if (!fs.is_open()) {
        throw std::runtime_error("cannot find json file! check the path");
    }

    std::string str;
    std::string tmp;
    while (getline(fs, tmp))
        str += tmp + '\n';
    fs.close();

    return str;
}

#ifdef TEST
// 测试，打印语法分析构建的ast
void printAst(simpleJson::JsonValue val) {
    switch (val.getType()) {
        case simpleJson::JsonType::Int:
            std::cout << val._data._int << std::endl;
            break;
        case simpleJson::JsonType::Float:
            std::cout << val._data._float << std::endl;
            break;
        case simpleJson::JsonType::Bool:
            std::cout << val._data._bool << std::endl;
            break;
        case simpleJson::JsonType::Null:
            std::cout << "null" << std::endl;
            break;
        case simpleJson::JsonType::String:
            std::cout << val._data._string << std::endl;
            break;
        case simpleJson::JsonType::Array:
            std::cout << "[\n";
            for (const auto& ele : val._data._array) {
                printAst(ele);
            }
            std::cout << "]" << std::endl;
            break;
        case simpleJson::JsonType::Object:
            std::cout << "{\n";
            for (const auto& ele : val._data._object) {
                std::cout << ele.first << " : ";
                printAst(ele.second);
            }
            std::cout << "}" << std::endl;
            break;
        default:
            ;
    }
}
#endif

void read_from_json() {
    try {
        std::string json(load_json());
        // std::string json("\n\"s\": \"\",\n \"ok\":\"1\"");
        // std::string json("{\n\"hello\": true,\n \"null\":[null, false,]\n}");
        simpleJson::Lexer le(json);
        le.peekToken();

        std::vector<std::string> ch;

        simpleJson::Parser pa(le);
        pa.parse();

        simpleJson::JsonValue val(pa.getAst());
        // printAst(val);
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
    }
}

int main() {
    read_from_json();
    return 0;
}
