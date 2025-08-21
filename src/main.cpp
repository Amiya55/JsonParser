#include "json.h"
#include "json_type.h"
#include "lexer_parser.h"
#include "utilities.h"

#include <exception>
#include <fstream>
#include <iostream>

namespace
{

using jValue = simple_json::JsonValue;
using jType = simple_json::JsonType;

std::string ReadTest()
{
    std::string result;

    const std::string json_path("tmp.json");
    std::fstream fs;
    fs.open(json_path, std::ios::in);
    if (!fs.is_open())
    {
        std::cout << "failed to open " << json_path << '\n';
    }

    std::string buffer;
    while (std::getline(fs, buffer))
    {
        result += buffer + '\n';
    }

    fs.close();
    return result;
}

void TestJsonTypes1()
{
    try
    {
        std::string str("hello world");
        std::vector<jValue> vec;
        jValue json(nullptr);
        std::cout << json.GetVal<jType::Null>() << '\n';

        json = 120.23;
        std::cout << json.GetVal<jType::Float>() << '\n';

        json = str;
        std::cout << json.GetVal<jType::String>() << '\n';
    }
    catch (std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }
}

void TestJsonTypes2()
{
    try
    {
        std::string str("hello world");
        jValue jsonstr1("dhello");
        jValue jsonstr2(str);
        std::cout << jsonstr1.GetVal<jType::String>() << '\n';
        std::cout << jsonstr2.GetVal<jType::String>() << '\n';

        jValue json(nullptr);
        jValue json2(12.34);
        jValue json3(12);

        jValue json4 = jValue::MakeArr({str, json2, false});
        std::vector<jValue> vec = json4.GetVal<jType::Array>();
        std::cout << vec[0].GetVal<jType::String>() << '\n';
        std::cout << vec[1].GetVal<jType::Float>() << '\n';
        std::cout << vec[2].GetVal<jType::Bool>() << '\n';

        jValue json5 = jValue::MakeObj({{"string", true}, {"hello", "world"}, {"12", 12.34}});
    }
    catch (std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }
}

void TestJsonTypes3()
{
    jValue jv1("json");
    jValue jv2(12.323);
    jValue jv3(100);
    jValue jv4 = jValue::MakeArr({jv1, jv2, jv3});
    jValue jv5 = jValue::MakeObj({{"first", 12}, {"second", nullptr}, {"third", jv4}});

    jValue jv6 = jValue::MakeObj({{"object", jv5}, {"array", jv4}});

    jValue jv7(nullptr);

    // std::cout << jv1 << '\n';
    // std::cout << jv2 << '\n';
    // std::cout << jv3 << '\n';
    // std::cout << jv4 << '\n';
    // std::cout << jv5 << '\n';
    // std::cout << jv6 << '\n';
    // std::cout << jv7 << '\n';

    jValue jv8(std::move(jv6));
    std::cout << jv8.GetType() << '\n';
    jv8 = jv2;
    std::cout << jv8.GetType() << '\n';
}

void ConvertUnicodeTest()
{
    // try {
    //     std::string str("9f000");
    //     unsigned int result = std::stoul(str, nullptr, 16);
    //     std::cout << result << '\n';
    // } catch (std::exception &e) {
    //     std::cerr << e.what() << '\n';
    // }

    std::string str("https://\\u4e00我知道这是\\uge中文字符\\u9fa5");
    std::string ret = simple_json::ConvertUnicodeEscape(str);
    std::cout << ret << '\n';
}

void LexerTest()
{
    try
    {
        std::string json_str("True");
        simple_json::Lexer lexer(json_str);
    }
    catch (const std::exception &e)
    {
        std::cout << e.what() << '\n';
    }

    // try
    // {
    //     std::string jsonStr("\"你好，我是谁？\\u4e00, hello world.\\u9ga5\\u4f00\"");
    //     std::string jsonStr(R"("\u4g00\u9fa5\u4f00")");
    //     simpleJson::Lexer lexer(jsonStr);
    // }
    // catch (const std::exception &e)
    // {
    //     std::cout << e.what() << '\n';
    // }

    // try
    // {
    //     std::string jsonStr("6.2e");
    //     simpleJson::Lexer lexer(jsonStr);
    // }
    // catch (const std::exception &e)
    // {
    //     std::cout << e.what() << '\n';
    // }
}

void ErrMsgsTest()
{
    try
    {
        // simpleJson::Lexer(R"({
        //     "literal": nulL,
        //     "boolean": False,
        //     "number": -0.23,
        //     "string: null
        // })");

        simple_json::Lexer lexer(ReadTest());
    }
    catch (const std::exception &e)
    {
        std::cout << e.what() << '\n';
    }
}

void LineSplitTest()
{
    std::cout << "行分割测试" << '\n';
    simple_json::Lexer lexer(R"({
        "number": 12e-2,
        "literal": true,
        "string": "json"
    })");
}

// token流查看
void TokenStreamTest()
{
    try
    {
        simple_json::Lexer lexer(ReadTest());
        const simple_json::JsonData &data = lexer.GetToken();
        for (const auto &token : data.tokens_)
        {
            std::cout << token.raw_value_ << '\n';
        }
    }
    catch (const std::exception &e)
    {
        std::cout << e.what() << '\n';
    }
}

// 语法分析器测试
void ParserTest()
{
    try
    {
        simple_json::Lexer lexer(ReadTest());
        simple_json::Parser parser(std::move(lexer.GetToken()));
    }
    catch (const std::exception &e)
    {
        std::cout << e.what() << '\n';
    }
}

// json测试
void JsonTest()
{
    simple_json::Json json = simple_json::Json::FromFile("./tmp.json");
}

} // namespace

int main()
{
    // std::cout << "hello JsonParser!" << '\n';
    // TestJsonTypes1();
    // TestJsonTypes2();
    // TestJsonTypes3();
    // ConvertUnicodeTest();
    // LexerTest();
    // ErrMsgsTest();
    // LineSplitTest();
    // TokenStreamTest();
    // ParserTest();
    JsonTest();
    return 0;
}
