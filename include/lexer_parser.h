#ifndef Lexer_Parser_H
#define Lexer_Parser_H

#include "config.h"
#include "json_type.h"

#include <cstddef>
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

    /**
     * @brief Adds an error description based on information in the ErrInfo struct for highlighted printing.
     *
     * @param errInfo Error information retrieved from various methods, including the error description, line and column
     * number of the error, and the highlight print length.
     */
    template <typename T, typename = std::enable_if_t<std::is_same_v<std::decay_t<T>, ErrInfo>>>
    void AddError(T &&errInfo)
    {
        errors_.push_back(std::forward<T>(errInfo));
    }

    /**
     * @brief Determines if any errors occurred during the entire program module's execution.
     *
     * @return Returns true if an error is found, and false if not.
     */
    [[nodiscard]] bool HasError() const noexcept;

    /**
     * @brief Throws all recorded error messages.
     *
     * @param throw_all Choose whether to print all errors. Set the parameter to true to print all errors, or false to
     * print a single error.
     */
    void ThrowError(bool throw_all = false) const;
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

    /**
     * @brief Retrieves the token stream analyzed by the lexical analyzer as well as the original JSON string.
     *
     * @return Returns the original JSON string and the token stream (i.e., the offset address of each line) obtained
     * during the lexical analysis.
     */
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

    /**
     * @brief Marks the start and end positions of each line in the original JSON string.
     *
     */
    void SplitLines() noexcept; // 标记原始json字符串中每行的起始位置和结束位置

    /**
     * @brief The entry point for the lexical analyzer. It tokenizes the original JSON string by breaking it down into a
     * stream of tokens.
     *
     */
    void Scan(); // 词法分析器入口，对原始json字符串进行切分，拆分为token流

    /**
     * @brief Checks if a token is terminated. A token terminates if the current character is a ,, ], }, \0, or :.
     *
     * @return Returns true if the token is terminated; otherwise, returns false.
     */
    [[nodiscard]] bool TokenIsOver() const noexcept; // 判断一个token是否结束

    /**
     * @brief Check for the end of the JSON string (encounters EOF).
     *
     * @return Returns true if the JSON string ends (encounters EOF), otherwise returns false.
     */
    [[nodiscard]] bool IsAtEnd() const noexcept;

    /**
     * @brief Check a line is ended or not. (encounters \n)
     *
     * @return Returns true if a line has ended (encounters \n); otherwise, returns false.
     */
    [[nodiscard]] bool IsEndOfLine() const noexcept; // 判断一行是否结束

    /**
     * @brief Get the character being parsed.
     *
     * @return return the current character.
     */
    [[nodiscard]] char Current() const noexcept;

    /**
     * @brief Advance the lexer by one character.
     *
     * @return Returns the advanced character.
     */
    char Advance() noexcept;

    /**
     * @brief Creates a new token from the information parsed by the lexical analyzer.
     *
     * @param str Raw string of the new token
     * @param type The C++ type corresponding to the new token
     * @return A new token object created from the basic information you passed.
     */
    [[nodiscard]] Token MakeToken(std::string &&str, TokenType type) const noexcept;

    // 以下函数都是_scan函数的子模块
    /**
     * @brief The lexical analyzer parses a JSON string.
     *
     * @param return_token A return-by-reference parameter used to return the successfully parsed string.
     * @param err_info A return-type parameter used to return the error information encountered during parsing (which
     * signifies a parsing failure).
     * @return Returns true on successfully parsing a token, false otherwise.
     */
    bool ParseString(Token &return_token, ErrInfo &err_info);  // 解析json字符串

    /**
     * @brief The lexical analyzer parses a JSON number.
     *
     * @param return_token A return-by-reference parameter used to return the successfully parsed number.
     * @param err_info A return-type parameter used to return the error information encountered during parsing (which
     * signifies a parsing failure).
     * @return Returns true on successfully parsing a token, false otherwise.
     */
    bool ParseNumber(Token &return_token, ErrInfo &err_info);  // 解析json数字

    /**
     * @brief The lexical analyzer parses a JSON literal (true, false, null).
     *
     * @param return_token A return-by-reference parameter used to return the successfully parsed literal.
     * @param err_info A return-type parameter used to return the error information encountered during parsing (which
     * signifies a parsing failure).
     * @return Returns true on successfully parsing a token, false otherwise.
     */
    bool ParseLiteral(Token &return_token, ErrInfo &err_info); // 解析json字面量(true, false, null)
};

class Parser
{
  public:
    template <typename T, typename = std::enable_if_t<std::is_same_v<std::decay_t<T>, JsonData>>>
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

    /**
     * @brief Lexer entry point
     *
     */
    void Parse() noexcept; // 词法分析器入口

    /**
     * @brief The main control function of the JSON lexical analyzer, used for parsing various JSON types.
     *
     * @param return_value A return-type parameter used to return the C++ type parsed by this function.
     * @return Returns true on successful parse, false otherwise.
     */
    [[nodiscard]] bool ParseValue(JsonValue &return_value) noexcept;

    /**
     * @brief Parse an json object from a token stream
     *
     * @return C++ hashmap structure returned by the parser.
     */
    [[nodiscard]] JsonValue ParseObject() noexcept;

    /**
     * @brief Parse an json array from a token stream
     *
     * @return C++ array structure returned by the parser.
     */
    [[nodiscard]] JsonValue ParseArray() noexcept;

    /**
     * @brief Parse a JSON number and dispatch it to a C++ integer or floating-point type.
     *
     * @return Returns a C++ integer or floating-point type encapsulated in a JsonValue class after parsing.
     */
    [[nodiscard]] JsonValue ParseNumber() noexcept; // json数字有很多种情况，要区分整形和浮点

    [[nodiscard]] const Token *Prev() const noexcept;

    /**
     * @brief View the current token
     *
     * @return Pointer to the current token
     */
    [[nodiscard]] const Token *Current() const noexcept;

    /**
     * @brief Look ahead one token
     *
     * @return Pointer to the token ahead
     */
    [[nodiscard]] const Token *Peek() const noexcept;

    Token *Advance() noexcept;
    /**
     * @brief Assert the specified token type
     *
     * @param token The token pointer you specified
     * @param token_type The type you asserted
     * @return Returns true on successful assertion, false otherwise.
     */
    [[nodiscard]] bool Consume(const Token *token,
                               TokenType token_type) noexcept; // 断言当前的token是什么类型，断言失败添加错误信息

    /**
     * @brief Constructing error messages
     *
     * @param err_desc Error Message
     * @param cur_token Token corresponding to the error location
     * @param highlight_pos Error highlight starting position. Only the starting column is needed, not the row, as the
     * cur_token parameter already specifies the error line number.
     * @param highlight_len Error highlight length.
     * If the highlight_pos and highlight_len parameters are not specified, the starting column and length of cur_token
     * will be used by default.
     */
    void MakeErrInfo(std::string err_desc, const Token *cur_token, size_t highlight_pos = 0,
                     size_t highlight_len = 0) noexcept;

    /**
     * @brief The parser enters panic mode and consumes tokens
     * until it finds a comma ,, or a bracket [, ], or a brace {, }.
     *
     */
    void Synchronize() noexcept;
};
} // namespace simple_json

#endif // Lexer_Parser_H
