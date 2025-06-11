#include <exception>
#include <iostream>
#include <fstream>
#include "iniParser.h"
#include "jsonParser.h"
#include "jsonTypes.h"
#include "sjApi.h"

#define TEST

#ifdef TEST
// 测试，打印语法分析构建的ast
void printAst(simpleJson::JsonValue val) {
    switch (val.getType()) {
        case simpleJson::JsonType::Int:
            std::cout << val.getInt() << std::endl;
            break;
        case simpleJson::JsonType::Float:
            std::cout << val.getFloat() << std::endl;
            break;
        case simpleJson::JsonType::Bool:
            std::cout << val.getBool() << std::endl;
            break;
        case simpleJson::JsonType::Null:
            std::cout << "null" << std::endl;
            break;
        case simpleJson::JsonType::String:
            std::cout << val.getString() << std::endl;
            break;
        case simpleJson::JsonType::Array:
            std::cout << "[\n";
            for (const auto &ele: val.getArray()) {
                printAst(ele);
            }
            std::cout << "]" << std::endl;
            break;
        case simpleJson::JsonType::Object:
            std::cout << "{\n";
            for (const auto &ele: val.getObject()) {
                std::cout << ele.first << " : ";
                printAst(ele.second);
            }
            std::cout << "}" << std::endl;
            break;
        default:
            ;
    }
}

std::string load_json(const std::string& path) {
    std::fstream fs;
    fs.open(path, std::ios::in);
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
        std::string json(load_json("data.json"));
        // std::string json("\n\"s\": \"\",\n \"ok\":\"1\"");
        // std::string json("{\n\"hello\": true,\n \"null\":[null, false,]\n}");
        simpleJson::Lexer le(json);

        simpleJson::Parser pa(le);
        pa.parse();

        simpleJson::JsonValue val(*pa.getAst());
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
    // val = {v1, v2, v3, v4, v5, v6};
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

void testAst() {
    try {
        std::string json = load_json("tmp.json");

        simpleJson::Lexer le(json);
        simpleJson::Parser pa(le);
        pa.parse();
        printAst(*pa.getAst());
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
    }
}

// sJson功能测试
void testSJson() {
    simpleJson::sJson json = simpleJson::sJson::array();
    // std::string s("hello world");
    json.push_back("hello world");
    json.push_back(100);
    json.push_back(false);
    json.push_back(123.456);
    json.push_back(nullptr);

    std::vector<simpleJson::JsonValue> v = {1,"hello world",false, nullptr};
    json.push_back(v);

    simpleJson::JsonValue jv({"hello world",false,nullptr});

    json.push_back({"Python", "C++", 123.456, {"hello world",false,nullptr}});

    printAst(*json.getRoot());

}

#endif

int main() {
    // read_from_json();
    // apiTest();
    // jsonValueTest();
    // testAst();
    testSJson();
    return 0;
}
