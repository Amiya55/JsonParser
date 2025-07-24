#ifndef Lexer_Parser_H
#define Lexer_Parser_H

#include <string>
#include <vector>

#include "config.h"

namespace simpleJson {
    enum class TokenType {
        LBRACE, RBRACE, LBRACKET, RBRACKET,
        STR, NUM, COMMA, COLON,
        TRUE, FALSE, NULL_
    };

    struct Token {
        std::string rawValue;
        TokenType type;

        // 记录token的位置
        POS_T row; // 所在行
        POS_T col; // 所在列
        LENGTH_T len; // Token长度
    };

    // 原始json字符串和token流(经过词法分析器处理后得到)
    // 因为后面的语法分析器还需要用原始json字符串和token流，
    // 所以我们将他们单独封装一下
    struct JsonData {
        std::string source; // 原始json字符串
        std::vector<POS_T> linesIndex; // 原始json字符串换行符偏移量

        std::vector<Token> tokens; // Token流
    };

    class Lexer {
    public:
        JsonData _data; // 当前json的所有信息，包括原始json字符串，json换行位置偏移，token流

        template<typename T, typename = enableIfString<T>>
        explicit Lexer(T&& source) {
            _data.source = std::forward<T>(source);
            _scan();
        }
        ~Lexer() = default;

        Lexer(const Lexer&) = delete;
        Lexer(Lexer&&) = delete;
        Lexer& operator=(const Lexer&) = delete;
        Lexer& operator=(Lexer&&) = delete;

    private:
        // Dfa基本状态
        enum class DfaStat {
            Start, Done, Error,

            InString, StringEscape, EndString,

            NumberSign, NumberZero, NumberIntegral, NumberFractionBegin, NumberFraction,
            NumberExponentBegin, NumberExponentSign, NumberExponent, NumberEnd,

            TrueT, TrueR, TrueU, TrueE,

            FalseF, FalseA, FalseL, FalseS, FalseE,

            NullN, NullU, NullL1, NullL2
        };

        POS_T _curIndex{0}; // 当前在原始json字符串中的索引
        POS_T _curRow{0}; // 当前字符行
        POS_T _curCol{0}; // 当前字符列

        void _scan();
        [[nodiscard]] bool _dfaDone(char curChar) const noexcept; // 判断一个token是否结束
        [[nodiscard]] bool _isAtEnd() const noexcept;
        [[nodiscard]] char _prev() const noexcept;
        [[nodiscard]] char _current() const noexcept;
        [[nodiscard]] char _peek() const noexcept;
        char _advance() noexcept;
        [[nodiscard]] Token _makeToken(std::string&& str, TokenType type) const noexcept;

        // 以下函数都是_scan函数的子模块
        bool _parseString(std::string& returnToken); // 解析json字符串
        bool _parseNumber(std::string& returnToken); // 解析json数字
        bool _parseLiteral(std::string& returnToken, TokenType& type); // 解析json字面量(true, false, null)
    };
}

#endif // Lexer_Parser_H
