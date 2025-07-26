#ifndef Lexer_Parser_H
#define Lexer_Parser_H

#include "config.h"
#include "json_type.h"

#include <map>
#include <string>
#include <type_traits>
#include <vector>

namespace simpleJson
{
enum class TokenType
{
    LBRACE,
    RBRACE,
    LBRACKET,
    RBRACKET,
    STR,
    NUM,
    COMMA,
    COLON,
    TRUE,
    FALSE,
    NULL_,
    EOF_
};

struct Token
{
    std::string rawValue;
    TokenType type;

    // 记录token的位置
    POS_T row;    // 所在行
    POS_T col;    // 所在列
    LENGTH_T len; // Token长度
};

// 原始json字符串和token流(经过词法分析器处理后得到)
// 因为后面的语法分析器还需要用原始json字符串和token流，
// 所以我们将他们单独封装一下
struct JsonData
{
    std::string source;                                  // 原始json字符串
    std::map<POS_T, std::pair<POS_T, POS_T>> linesIndex; // 原始json字符串每一行的起始和结束偏移量

    std::vector<Token> tokens; // Token流
};

struct ErrInfo
{
    std::string errDesc;     // 错误描述
    std::string currentLine; // 错误所在行

    POS_T row;
    POS_T col;
    LENGTH_T len; // 错误token长度，便于高亮打印
};

// 这个类用来存储词法，语法分析中检测到的各种错误
class ErrReporter
{
    std::vector<ErrInfo> _errors;

  public:
    ErrReporter() = default;
    ~ErrReporter() = default;

    ErrReporter(const ErrReporter &) = delete;
    ErrReporter(ErrReporter &&) = delete;
    ErrReporter &operator=(const ErrReporter &) = delete;
    ErrReporter &operator=(ErrReporter &&) = delete;

    template <typename T, typename = std::enable_if_t<std::is_same_v<std::decay_t<T>, ErrInfo>>>
    void addError(T &&errInfo)
    {
        _errors.push_back(std::forward<T>(errInfo));
    }

    [[nodiscard]] bool hasError() const noexcept;
    void throwError(bool throwAll = false) const;
};

class Lexer
{
  public:
    JsonData _data; // 当前json的所有信息，包括原始json字符串，json换行位置偏移，token流

    template <typename T, typename = enableIfString<T>> explicit Lexer(T &&source)
    {
        _data.source = std::forward<T>(source);
        _splitLines();
        _scan();

        if (_errReporter.hasError())
        {
            _errReporter.throwError(true);
        }
    }
    ~Lexer() = default;

    Lexer(const Lexer &) = delete;
    Lexer(Lexer &&) = delete;
    Lexer &operator=(const Lexer &) = delete;
    Lexer &operator=(Lexer &&) = delete;

  private:
    ErrReporter _errReporter;

    // Dfa基本状态
    enum class StringDfaStat
    {
        STRING_START,
        IN_STRING,
        STRING_END,
        STRING_ESCAPE,
        STRING_UNICODE_START,
        ERROR
    };

    enum class NumberDfaStat
    {
        NUMBER_START,
        NUMBER_SIGN,
        NUMBER_ZERO,
        NUMBER_INTEGRAL,
        NUMBER_FRACTION_BEGIN,
        NUMBER_FRACTION,
        NUMBER_EXPONENT_BEGIN,
        NUMBER_EXPONENT_SIGN,
        NUMBER_EXPONENT,
        NUMBER_END,
        ERROR
    };

    enum class LiteralDfaStat
    {
        LITERAL_START,
        LITERAL_END,
        ERROR,
        TRUE_T,
        TRUE_R,
        TRUE_U,
        TRUE_E,
        FALSE_F,
        FALSE_A,
        FALSE_L,
        FALSE_S,
        FALSE_E,
        NULL_N,
        NULL_U,
        NULL_L1,
        NULL_L2
    };

    POS_T _curIndex{0}; // 当前在原始json字符串中的索引
    POS_T _curRow{0};   // 当前字符行
    POS_T _curCol{0};   // 当前字符列

    void _splitLines() noexcept; // 标记原始json字符串中每行的起始位置和结束位置
    void _scan();

    [[nodiscard]] bool _tokenIsOver() const noexcept; // 判断一个token是否结束
    [[nodiscard]] bool _isAtEnd() const noexcept;
    [[nodiscard]] bool _isEndOfLine() const noexcept; // 判断一行是否结束

    [[nodiscard]] char _current() const noexcept;
    char _advance() noexcept;

    [[nodiscard]] Token _makeToken(std::string &&str, TokenType type) noexcept;

    // 以下函数都是_scan函数的子模块
    bool _parseString(Token &returnToken, ErrInfo &errInfo);  // 解析json字符串
    bool _parseNumber(Token &returnToken, ErrInfo &errInfo);  // 解析json数字
    bool _parseLiteral(Token &returnToken, ErrInfo &errInfo); // 解析json字面量(true, false, null)
};

class Parser
{
  public:
    JsonValue _json;
    JsonData _jsonData;

    template <typename T, typename = std::enable_if_t<std::is_same_v<std::decay<T>, JsonData>>>
    explicit Parser(T &&jsonData) : _jsonData(std::forward<T>(jsonData))
    {
        _parse();
    }
    ~Parser() = default;

    Parser(const Parser &) = delete;
    Parser(Parser &&) = delete;
    Parser &operator=(const Parser &) = delete;
    Parser &operator=(const Parser &&) = delete;

  private:
    ErrReporter _errReporter;

    void _parse() const noexcept; // 词法分析器入口
    void _parseValue() const noexcept;
    void _parseObject() const noexcept;
    void _parseArray() const noexcept;

    [[nodiscard]] const Token &peek() const noexcept;
    const Token &_advance() const noexcept;
};
} // namespace simpleJson

#endif // Lexer_Parser_H
