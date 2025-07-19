#include <exception>
#include <iostream>

#include "JsonTypes.h"

using jValue = simpleJson::JsonValue;
using jType = simpleJson::JsonType;

void test_json_types1() {
    try {
        std::string str("hello world");
        std::vector<jValue> vec;
        jValue json(nullptr);
        std::cout << json.getVal<jType::Null>() << std::endl;

        json = 120.23;
        std::cout << json.getVal<jType::Float>() << std::endl;

        json = str;
        std::cout << json.getVal<jType::String>() << std::endl;
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
    }
}

void test_json_types2() {
    try {
        std::string str("hello world");
        jValue jsonstr1("dhello");
        jValue jsonstr2(str);
        std::cout << jsonstr1.getVal<jType::String>() << std::endl;
        std::cout << jsonstr2.getVal<jType::String>() << std::endl;


        jValue json(nullptr);
        jValue json2(12.34);
        jValue json3(12);

        jValue json4 = jValue::makeArr({
            str, json2, false
        });
        std::vector<jValue> vec = json4.getVal<jType::Array>();
        std::cout << vec[0].getVal<jType::String>() << std::endl;
        std::cout << vec[1].getVal<jType::Float>() << std::endl;
        std::cout << vec[2].getVal<jType::Bool>() << std::endl;

        jValue json5 = jValue::makeObj({
            {"string", true}, {"hello", "world"}, {"12", 12.34}
        });
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
    }
}

int main() {
    // std::cout << "hello JsonParser!" << std::endl;
    // test_json_types1();
    test_json_types2();
    return 0;
}
