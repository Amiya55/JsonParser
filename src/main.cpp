#include <exception>
#include <iostream>
#include <fstream>
#include "iniParser.h"
#include "jsonParser.h"
#include "jsonTypes.h"
#include "sjApi.h"
#include "utilities.h"

// #define TEST

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

void read_from_json() {
    try {
        std::string json(load_json());
        // std::string json("\n\"s\": \"\",\n \"ok\":\"1\"");
        // std::string json("{\n\"hello\": true,\n \"null\":[null, false,]\n}");
        simpleJson::Lexer le(json);

        simpleJson::Parser pa(le);
        pa.parse();

        simpleJson::JsonValue val(pa.getAst());
        // printAst(val);
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
    }
}

void apiTest() {
    try {
        const std::string path("data.json");
        simpleJson::sJson::fromFile(path);
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
    }
}

void jsonValueTest() {
    simpleJson::JsonValue val;

    val = 10ll;
    std::cout << val.getInt() << std::endl;

    // val = std::string("hello world");
    val = "hello world";
    std::cout << val.getString() << std::endl;

    simpleJson::JsonValue v1(1ll);
    simpleJson::JsonValue v2(123.456);
    simpleJson::JsonValue v3(3ll);
    simpleJson::JsonValue v4("hello world");
    simpleJson::JsonValue v5(false);
    simpleJson::JsonValue v6(true);

    // std::vector<simpleJson::JsonValue> vec = {v1, v2, v3, v4, v5, v6};
    // val = std::move(vec);
    val = {v1, v2, v3, v4, v5, v6};
    for (simpleJson::JsonValue &ele: val.getArray()) {
        switch (ele.getType()) {
            case simpleJson::JsonType::Int:
                std::cout << ele.getInt() << " ";
                break;
            case simpleJson::JsonType::Float:
                std::cout << ele.getFloat() << " ";
                break;
            case simpleJson::JsonType::String:
                std::cout << ele.getString() << " ";
                break;
            case simpleJson::JsonType::Bool:
                std::cout << ele.getBool() << " ";
            default:
                ;
        }
    }
    std::cout << std::endl;
}

int main() {
    // read_from_json();
    // apiTest();
    jsonValueTest();
    return 0;
}
