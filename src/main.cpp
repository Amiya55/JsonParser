#include <exception>
#include <iostream>

#include "json_type.h"
#include "utilities.h"

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

void test_json_types3() {
    jValue jv1("json");
    jValue jv2(12.323);
    jValue jv3(100);
    jValue jv4 = jValue::makeArr({jv1, jv2, jv3});
    jValue jv5 = jValue::makeObj({
        {"first", 12},
        {"second", nullptr},
        {"third", jv4}
    });

    jValue jv6 = jValue::makeObj({
        {"object", jv5},
        {"array", jv4}
    });

    jValue jv7(nullptr);

    // std::cout << jv1 << std::endl;
    // std::cout << jv2 << std::endl;
    // std::cout << jv3 << std::endl;
    // std::cout << jv4 << std::endl;
    // std::cout << jv5 << std::endl;
    // std::cout << jv6 << std::endl;
    // std::cout << jv7 << std::endl;

    jValue jv8(std::move(jv6));
    std::cout << jv8.getType() << std::endl;
    jv8 = jv2;
    std::cout << jv8.getType() << std::endl;
}

void convert_unicode_test() {
    // try {
    //     std::string str("9f000");
    //     unsigned int result = std::stoul(str, nullptr, 16);
    //     std::cout << result << std::endl;
    // } catch (std::exception &e) {
    //     std::cerr << e.what() << std::endl;
    // }

    std::string str("https://\\u4e00我知道这是\\uge中文字符\\u9fa5");
    std::string ret = simpleJson::convert_unicode_escape(str);
    std::cout << ret << std::endl;
}

int main() {
    // std::cout << "hello JsonParser!" << std::endl;
    // test_json_types1();
    // test_json_types2();
    // test_json_types3();
    convert_unicode_test();
    return 0;
}
