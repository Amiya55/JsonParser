#ifndef Lexer_Parser_H
#define Lexer_Parser_H

#include "config.h"
#include "json_type.h"

#include <cstdint>
#include <map>
#include <string>
#include <type_traits>
#include <vector>

namespace simple_json
{
enum class TokenType : uint8_t
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
    std::string raw_value_;
    TokenType type_;

    // 记录token的位置
    POS_T row_;    // 所在行
    POS_T col_;    // 所在列
    LENGTH_T len_; // Token长度
};

// 原始json字符串和token流(经过词法分析器处理后得到)
// 因为后面的语法分析器还需要用原始json字符串和token流，
// 所以我们将他们单独封装一下
struct JsonData
{
    std::string source_;                                   // 原始json字符串
    std::map<POS_T, std::pair<POS_T, POS_T>> lines_index_; // 原始json字符串每一行的起始和结束偏移量

    std::vector<Token> tokens_; // Token流
};

struct ErrInfo
{
    std::string err_desc_;     // 错误描述
    std::string current_line_; // 错误所在行

    POS_T row_;
    POS_T col_;
    LENGTH_T len_; // 错误token长度，便于高亮打印
};

// 这个类用来存储词法，语法分析中检测到的各种错误
class ErrReporter
{
    std::vector<ErrInfo> errors_;

  public:
    ErrReporter() = default;
    ~ErrReporter() = default;

    ErrReporter(const ErrReporter &) = delete;
    ErrReporter(ErrReporter &&) = delete;
    ErrReporter &operator=(const ErrReporter &) = delete;
    ErrReporter &operator=(ErrReporter &&) = delete;

    template <typename T, typename = std::enable_if_t<std::is_same_v<std::decay_t<T>, ErrInfo>>>
    void AddError(T &&errInfo)
    {
        errors_.push_back(std::forward<T>(errInfo));
    }

    [[nodiscard]] bool HasError() const noexcept;
    void ThrowError(bool throwAll = false) const;
};

class Lexer
{
  public:
    template <typename T, typename = enableIfString<T>> explicit Lexer(T &&source)
    {
        data_.source_ = std::forward<T>(source);
        SplitLines();
        Scan();

        if (err_reporter_.HasError())
        {
            err_reporter_.ThrowError(true);
        }
    }
    ~Lexer() = default;

    Lexer(const Lexer &) = delete;
    Lexer(Lexer &&) = delete;
    Lexer &operator=(const Lexer &) = delete;
    Lexer &operator=(Lexer &&) = delete;

    [[nodiscard]] JsonData &GetToken() noexcept;

  private:
    JsonData data_;            // 当前json的所有信息，包括原始json字符串，json换行位置偏移，token流
    ErrReporter err_reporter_; // 错误处理模块

    // Dfa基本状态
    enum class StringDfaStat : uint8_t
    {
        STRING_START,
        IN_STRING,
        STRING_END,
        STRING_ESCAPE,
        STRING_UNICODE_START,
        ERROR
    };

    enum class NumberDfaStat : uint8_t
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

    enum class LiteralDfaStat : uint8_t
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

    POS_T cur_index_{0}; // 当前在原始json字符串中的索引
    POS_T cur_row_{0};   // 当前字符行
    POS_T cur_col_{0};   // 当前字符列

    void SplitLines() noexcept; // 标记原始json字符串中每行的起始位置和结束位置
    void Scan();

    [[nodiscard]] bool TokenIsOver() const noexcept; // 判断一个token是否结束
    [[nodiscard]] bool IsAtEnd() const noexcept;
    [[nodiscard]] bool IsEndOfLine() const noexcept; // 判断一行是否结束

    [[nodiscard]] char Current() const noexcept;
    char Advance() noexcept;

    [[nodiscard]] Token MakeToken(std::string &&str, TokenType type) const noexcept;

    // 以下函数都是_scan函数的子模块
    bool ParseString(Token &return_token, ErrInfo &err_info);  // 解析json字符串
    bool ParseNumber(Token &return_token, ErrInfo &err_info);  // 解析json数字
    bool ParseLiteral(Token &return_token, ErrInfo &err_info); // 解析json字面量(true, false, null)
};

class Parser
{
  public:
    template <typename T, typename = std::enable_if_t<std::is_same_v<std::decay<T>, JsonData>>>
    explicit Parser(T &&json_data) : json_data_(std::forward<T>(json_data))
    {
        Parse();

        // 如果有错误信息，直接抛异常
        if (err_reporter_.HasError())
        {
            err_reporter_.ThrowError(true);
        }
    }
    ~Parser() = default;

    Parser(const Parser &) = delete;
    Parser(Parser &&) = delete;
    Parser &operator=(const Parser &) = delete;
    Parser &operator=(const Parser &&) = delete;

  private:
    JsonValue json_;           // 经过语法分析器构建的json数据结构
    JsonData json_data_;       // 从词法分析器拿到的json原始字符窜，token流和行偏移量
    ErrReporter err_reporter_; // 错误处理模块

    size_t cur_token_index_{0};

    void Parse() noexcept; // 词法分析器入口
    [[nodiscard]] JsonValue ParseValue() noexcept;
    [[nodiscard]] JsonValue ParseObject() noexcept;
    [[nodiscard]] JsonValue ParseArray() noexcept;

    [[nodiscard]] const Token *Current() const noexcept;
    [[nodiscard]] const Token *Advance() noexcept;
    [[nodiscard]] bool Consume(TokenType token_type,
                               std::string err_desc) noexcept; // 断言当前的token是什么类型，断言失败添加错误信息
};
} // namespace simple_json

#endif // Lexer_Parser_H
