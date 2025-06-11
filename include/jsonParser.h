#ifndef JSONPARSER_H
#define JSONPARSER_H

#include <string>
#include <vector>
#include "jsonTypes.h"

namespace simpleJson {
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

        Token(const Token &other) = default;

        Token &operator=(const Token &other) = default;

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

        Lexer(Lexer &other) = delete;

        Lexer(Lexer &&other) = delete;

        Lexer &operator=(Lexer &other) = delete;

        Lexer &operator=(Lexer &&other) = delete;

        const std::vector<Token> &getTokens() const noexcept;

        const std::vector<std::string> &getInput() const noexcept;

    private:
        void _splitToken(); // 遍历json字符串，构建token数组; 可能抛出invalid_argument异常

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
        const Lexer &_lexer;
        size_t _curIndex;
        Token _curToken;

        std::shared_ptr<JsonValue> _ast; // 抽象语法树

    public:
        explicit Parser(const Lexer &lexer) noexcept;

        Parser(Parser &other) = delete;

        Parser(Parser &&other) = delete;

        Parser &operator=(Parser &other) = delete;

        Parser &operator=(Parser &&other) = delete;

        void parse(); // 开始进行语法解析
        std::shared_ptr<JsonValue> getAst() noexcept;

    private:
        JsonValue _parseValue(); // 解析json值
        JsonValue _parseObject(); // 解析json对象
        JsonValue _parseArray(); // 解析json数组

        const Token &_peekPrev() const;

        const Token &_peek() const;

        const Token &_peekNext() const;

        void _advance(); // 向前探测一个token
        bool _match(const Token &token, TokenType type) const noexcept;

        bool _isAtEnd() const noexcept;

        std::string _buildErrMsg(
            std::string &&msg,
            const Token &highlightObj, size_t offset = 0) const noexcept; // 构建错误信息，高亮错误
    };
}

#endif // JSONPARSER_H
