#ifndef Lexer_Parser_H
#define Lexer_Parser_H

#include <string>
#include <vector>

#include "Config.h"

namespace simpleJson {
    enum class TokenType {
        LBRACE, RBRACE, LBRACKET, RBRACKET,
        STR, NUM, COMMA, COLON, _NULL
    };

    struct Token {
        std::string _rawValue;
        TokenType _type;

        // 记录token的位置
        POS_T _row; // 所在行
        POS_T _col; // 所在列
        LENGTH_T _len; // Token长度
    };

    // 原始json字符串和token流(经过词法分析器处理后得到)
    // 因为后面的语法分析器还需要用原始json字符串和token流，
    // 所以我们将他们单独封装一下
    struct JsonData {
        std::string _source; // json字符串
        std::vector<Token> _tokens; // Token流
    };

    class Lexer {
    public:
        JsonData _data;

        template<typename T, typename = enableIfString<T>>
        explicit Lexer(T&& source) {
            _data._source = std::forward<T>(source);
        }

        Lexer(const Lexer&) = delete;
        Lexer(Lexer&&) = delete;
        Lexer& operator=(const Lexer&) = delete;
        Lexer& operator=(Lexer&&) = delete;

    private:
    };
}

#endif // Lexer_Parser_H
