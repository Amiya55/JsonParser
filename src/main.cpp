#include "json_type.h"
#include "lexer_parser.h"
#include "utilities.h"

#include <exception>
#include <fstream>
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#endif

using jValue = simpleJson::JsonValue;
using jType = simpleJson::JsonType;

std::string read_test()
{
    std::string result;

    const std::string jsonPath("tmp.json");
    std::fstream fs;
    fs.open(jsonPath, std::ios::in);
    if (!fs.is_open())
    {
        std::cout << "failed to open " << jsonPath << '\n';
    }

    std::string buffer;
    while (std::getline(fs, buffer))
    {
        result += buffer + '\n';
    }

    fs.close();
    return result;
}

void test_json_types1()
{
    try
    {
        std::string str("hello world");
        std::vector<jValue> vec;
        jValue json(nullptr);
        std::cout << json.getVal<jType::Null>() << '\n';

        json = 120.23;
        std::cout << json.getVal<jType::Float>() << '\n';

        json = str;
        std::cout << json.getVal<jType::String>() << '\n';
    }
    catch (std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }
}

void test_json_types2()
{
    try
    {
        std::string str("hello world");
        jValue jsonstr1("dhello");
        jValue jsonstr2(str);
        std::cout << jsonstr1.getVal<jType::String>() << '\n';
        std::cout << jsonstr2.getVal<jType::String>() << '\n';

        jValue json(nullptr);
        jValue json2(12.34);
        jValue json3(12);

        jValue json4 = jValue::makeArr({str, json2, false});
        std::vector<jValue> vec = json4.getVal<jType::Array>();
        std::cout << vec[0].getVal<jType::String>() << '\n';
        std::cout << vec[1].getVal<jType::Float>() << '\n';
        std::cout << vec[2].getVal<jType::Bool>() << '\n';

        jValue json5 = jValue::makeObj({{"string", true}, {"hello", "world"}, {"12", 12.34}});
    }
    catch (std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }
}

void test_json_types3()
{
    jValue jv1("json");
    jValue jv2(12.323);
    jValue jv3(100);
    jValue jv4 = jValue::makeArr({jv1, jv2, jv3});
    jValue jv5 = jValue::makeObj({{"first", 12}, {"second", nullptr}, {"third", jv4}});

    jValue jv6 = jValue::makeObj({{"object", jv5}, {"array", jv4}});

    jValue jv7(nullptr);

    // std::cout << jv1 << '\n';
    // std::cout << jv2 << '\n';
    // std::cout << jv3 << '\n';
    // std::cout << jv4 << '\n';
    // std::cout << jv5 << '\n';
    // std::cout << jv6 << '\n';
    // std::cout << jv7 << '\n';

    jValue jv8(std::move(jv6));
    std::cout << jv8.getType() << '\n';
    jv8 = jv2;
    std::cout << jv8.getType() << '\n';
}

void convert_unicode_test()
{
    // try {
    //     std::string str("9f000");
    //     unsigned int result = std::stoul(str, nullptr, 16);
    //     std::cout << result << '\n';
    // } catch (std::exception &e) {
    //     std::cerr << e.what() << '\n';
    // }

    std::string str("https://\\u4e00我知道这是\\uge中文字符\\u9fa5");
    std::string ret = simpleJson::convert_unicode_escape(str);
    std::cout << ret << '\n';
}

void lexer_test()
{
    std::string jsonStr("True");
    simpleJson::Lexer lexer(jsonStr);

    // std::string jsonStr("\"你好，我是谁？\\u4e00, hello world.\\u9ga5\\u4f00\"");
    // std::string jsonStr(R"("\u4g00\u9fa5\u4f00")");
    // simpleJson::Lexer lexer(jsonStr);

    // std::string jsonStr("6.2e");
    // simpleJson::Lexer lexer(jsonStr);
}

void errMsgs_test()
{
    try
    {
        // simpleJson::Lexer(R"({
        //     "literal": nulL,
        //     "boolean": False,
        //     "number": -0.23,
        //     "string: null
        // })");

        simpleJson::Lexer lexer(read_test());
    }
    catch (const std::exception &e)
    {
        std::cout << e.what() << '\n';
    }
}

void line_split_test()
{
    std::cout << "行分割测试" << '\n';
    simpleJson::Lexer lexer(R"({
        "number": 12e-2,
        "literal": true,
        "string": "json"
    })");
}

// token流查看
void token_stream_test()
{
    try
    {
        simpleJson::Lexer lexer(read_test());
        simpleJson::JsonData &data = lexer._data;
        for (const auto &token : data.tokens)
        {
            std::cout << token.rawValue << '\n';
        }
    }
    catch (const std::exception &e)
    {
        std::cout << e.what() << '\n';
    }
}

int main()
{
#ifdef _WIN32
    SetConsoleOutputCP(65001);
#endif
    // std::cout << "hello JsonParser!" << '\n';
    // test_json_types1();
    // test_json_types2();
    // test_json_types3();
    // convert_unicode_test();
    // lexer_test();
    // errMsgs_test();
    // line_split_test();
    token_stream_test();
    return 0;
}
