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

    // 这个类用来存储词法，语法分析中检测到的各种错误
    class ErrMsgs {
        struct errInfo {
            // 错误地点的上下文，可以通过函数参数选择打印或者不打印错误上下文
            std::string currentLine;
            std::string prevLine;
            std::string nextLine;

            std::string errDesc; // 错误描述
            Token errToken; // 出错token
        };

        std::vector<errInfo> _messages;
    public:
        ErrMsgs() = default;
        ~ErrMsgs() = default;

        ErrMsgs(const ErrMsgs&) = delete;
        ErrMsgs(ErrMsgs&&) = delete;
        ErrMsgs& operator=(const ErrMsgs&) = delete;
        ErrMsgs& operator=(ErrMsgs&&) = delete;

        [[nodiscard]] bool hasError() const noexcept;
        void addError(std::string &&prevLine, std::string &&curLine,
                      std::string &&nextLine, std::string &&errDesc, Token &&errToken);

        void throwError(bool throwAll = false) const;
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
        ErrMsgs _errMsgs;

        // Dfa基本状态
        enum class StringDfaStat {
            STRING_START, IN_STRING, STRING_END,
            STRING_ESCAPE, STRING_UNICODE_START, ERROR
        };

        enum class NumberDfaStat {
            NUMBER_START, NUMBER_SIGN, NUMBER_ZERO, NUMBER_INTEGRAL,
            NUMBER_FRACTION_BEGIN, NUMBER_FRACTION, NUMBER_EXPONENT_BEGIN,
            NUMBER_EXPONENT_SIGN, NUMBER_EXPONENT, NUMBER_END, ERROR
        };

        enum class LiteralDfaStat {
            LITERAL_START, LITERAL_END, ERROR,
            TRUE_T, TRUE_R, TRUE_U, TRUE_E,
            FALSE_F, FALSE_A, FALSE_L, FALSE_S, FALSE_E,
            NULL_N, NULL_U, NULL_L1, NULL_L2
        };

        POS_T _curIndex{0}; // 当前在原始json字符串中的索引
        POS_T _curRow{0}; // 当前字符行
        POS_T _curCol{0}; // 当前字符列

        void _scan();

        [[nodiscard]] bool _tokenIsOver() const noexcept; // 判断一个token是否结束
        [[nodiscard]] bool _isAtEnd() const noexcept;
        [[nodiscard]] bool _isEndOfLine() const noexcept; // 判断一行是否结束

        [[nodiscard]] char _current() const noexcept;
        [[nodiscard]] char _peek() const noexcept;
        char _advance() noexcept;

        [[nodiscard]] static Token _makeToken(std::string&& str, TokenType type, POS_T row, POS_T col) noexcept;

        // 以下函数都是_scan函数的子模块
        bool _parseString(std::string& returnToken, std::string& errInfo); // 解析json字符串
        bool _parseNumber(std::string& returnToken, std::string& errInfo); // 解析json数字
        bool _parseLiteral(std::string& returnToken, std::string& errInfo, TokenType& type); // 解析json字面量(true, false, null)
    };
}

#endif // Lexer_Parser_H
