#ifndef Lexer_Parser_H
#define Lexer_Parser_H

#include <string>
#include <vector>

#include "config.h"

namespace simpleJson {
    enum class TokenType {
        LBRACE, RBRACE, LBRACKET, RBRACKET,
        STR, NUM, COMMA, COLON, NULL_
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
        // 位置溢出，用于判断_scan函数的子模块是否解析失败
        constexpr static POS_T POS_OVERGLOW = static_cast<POS_T>(-1);

        // Dfa基本状态
        enum DfaStat {
            Start, Done, Error,

            InString, StringEscape,

            NumberSign, NumberZero, NumberIntegral, NumberFractionBegin, NumberFraction,
            NumberExponentBegin, NumberExponentSign, NumberExponent,

            TrueT, TrueR, TrueU, TrueE,

            FalseF, FalseA, FalseL, FalseS, FalseE,

            NullN, NullU, NullL1, NullL2
        };

        void _scan();
        // 以下函数都是_scan函数的子模块
        POS_T _parseObject(); // 解析json对象
        POS_T _parseArray(); // 解析json数组
        POS_T _parseString(); // 解析json字符串
        POS_T _parseNumber(); // 解析json数字
        POS_T _parseLiteral(); // 解析json字面量(true, false, null)
    };
}

#endif // Lexer_Parser_H
