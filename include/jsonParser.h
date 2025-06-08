#ifndef JSONPARSER_H
#define JSONPARSER_H

#include <fstream>
#include <string>
#include <vector>
#include "jsonTypes.h"

namespace simpleJson {
#if __cplusplus >= 201703L
    enum class TokenType {
        LBRACE, RBRACE, LBRACKET, RBRACKET,
        COMMA, COLON, STRING, NUMBER, TRUE, FALSE, NULL_
    };

    struct Token {
        TokenType _type;
        std::string _value;

        // 错误定位
        size_t _line;
        size_t _column;

        Token(TokenType type, const std::string &value,
              size_t line, size_t column);

        TokenType type() const;
    };

    /* lexical analyser */
    class Lexer {
        std::vector<Token> _tokens; // 通过词法分析器分析出来的token数组

        std::vector<std::string> _input; // 通过二维数组，方便后面精确打印错误

        size_t _line = 0;
        size_t _column = 0;

    public:
        explicit Lexer(const std::string &input); // 可能抛出invalid_argument异常

        void peekToken(); // 遍历json字符串，构建token数组; 可能抛出invalid_argument异常
        std::vector<Token> getTokens() noexcept;

    private:
        void _skipWhitespace() noexcept;

        Token _parseString(); // 检测字符串; 可能抛出invalid_argument异常
        Token _parseNumber(); // 检测数字: 整数，浮点，负数，科学计数法; 可能抛出invalid_argument异常
        Token _parseKeyword(); // 检测是否为true, false, null; 可能抛出invalid_argument异常

        std::string _buildErrMsg(
            std::string &&msg, size_t line, // 构造异常信息，高亮显示错误位置
            size_t highlightBegin, size_t highlightEnd) const noexcept;
    };


    /* syntax analyser */
    class Parser {
        std::vector<Token> _tokens;
        size_t _curIndex;

        Token _curToken;

    public:
        explicit Parser(const std::vector<Token> &tokens) noexcept;

        void parse(); // 开始进行语法解析

    private:
        void _parseObject(); // 解析json对象
        void _parseArray(); // 解析json数组

        void _consume(TokenType expected, std::string&& errMsg); // 预测值，如果值不符合，抛异常
        Token &_advance(); // 向前探测一个token

        std::string _buildErrMsg(std::string&& msg) const noexcept; // 构建错误信息，高亮错误
    };

#else  // #if __cplusplus >= 201703L

    class JsonSyntaxChecker {
    public:
        JsonSyntaxChecker() = default;
        ~JsonSyntaxChecker() = default;

        void syntax_check(const std::string &jsonStr,
                          JsonValue::Type containerType); // may throw exception: runtime_error

    private:
        // check the key in json object
        void _check_key(const std::string &key); // may throw exception: runtime_error
        // check the value in json object, also it can check the element in json array
        void _check_value(const std::string &value); // may throw exception: runtime_error
    };

    class JsonFile {
    public:
        JsonFile();
        ~JsonFile();

        void open_json(const char *fileName); // may throw exception: runtime_error
        void close_json() noexcept;
        bool is_open() noexcept;

    private:
        std::string _fileName;
        std::fstream _file;

        std::string _jsonStr; // the json data, raw str data from the .json file
        JsonValue *_jsonData; // the json data, maybe json object or json array

        // transform the json str into our json classes and get ready to operate
        void _parse_data(JsonValue *mountPoint, std::string &strs) noexcept;
    };

#endif  // #if __cplusplus >= 201703L
}

#endif // JSONPARSER_H
