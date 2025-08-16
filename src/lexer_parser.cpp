#include "lexer_parser.h"
#include "config.h"
#include "json_type.h"
#include "utilities.h"

#include <cctype>
#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <string>

namespace simple_json 
{
bool ErrReporter::HasError() const noexcept
{
    return !errors_.empty();
}

void ErrReporter::ThrowError(const bool throwAll) const
{
    if (!HasError())
    {
        return;
    }

    std::string error_print_info;
    const size_t error_count = throwAll ? errors_.size() : 1;
    for (size_t i = 0; i < error_count; i++)
    {
        const POS_T err_row = errors_[i].row_ + 1; // 错误所在行的行号，面对用户，索引从1开始，所以加1
        const POS_T err_col = errors_[i].col_;

        // 构建错误所在行信息
        std::string err_info("[Row: " + std::to_string(err_row) + ", Col: " + std::to_string(err_col) + "] " +
                             errors_[i].err_desc_ + "\n");
        std::string line_info(std::to_string(err_row) + " | " + errors_[i].current_line_);
        if (line_info.back() != '\n')
        {
            // 如果末尾没有\n，则自动添加一个，否则可能造成打印格式错误
            line_info.push_back('\n');
        }

        error_print_info.append(err_info);
        error_print_info.append(line_info);

        // 构建空格缩进
        // 下面的err_row是行号长度，数字3为字符串" | "长度，errors_[i].col_为本行距离错误token首字符的长度
        std::string indent(std::to_string(err_row).length() + 3 + errors_[i].col_, ' ');
        // 高亮错误token
        std::string highlight(errors_[i].len_, '~');
        error_print_info.append(indent + highlight);

        // 构建分隔符
        error_print_info.append("\n- - - - - - - - - - -\n");
    }

    throw std::runtime_error(error_print_info);
}

JsonData &Lexer::GetToken() noexcept
{
    return data_;
}

void Lexer::SplitLines() noexcept
{
    POS_T begin = 0;
    POS_T cur_line_index = 0;
    for (POS_T end = 0; end < data_.source_.length(); ++end)
    {
        if (data_.source_[end] == '\n' || end == data_.source_.length() - 1)
        {
            data_.lines_index_[cur_line_index] = std::make_pair(begin, end + 1);
            begin = end + 1;
            ++cur_line_index;
        }
    }
}

void Lexer::Scan()
{
    for (cur_index_ = 0; cur_index_ < data_.source_.length();)
    {
        switch (data_.source_[cur_index_])
        {
        case '{':
            data_.tokens_.push_back(MakeToken("{", TokenType::LBRACE));
            Advance();
            break;
        case '}':
            data_.tokens_.push_back(MakeToken("}", TokenType::RBRACE));
            Advance();
            break;
        case '[':
            data_.tokens_.push_back(MakeToken("[", TokenType::LBRACKET));
            Advance();
            break;
        case ']':
            data_.tokens_.push_back(MakeToken("]", TokenType::RBRACKET));
            Advance();
            break;
        case ',':
            data_.tokens_.push_back(MakeToken(",", TokenType::COMMA));
            Advance();
            break;
        case ':':
            data_.tokens_.push_back(MakeToken(":", TokenType::COLON));
            Advance();
            break;
        case '\"': {
            Token return_token;
            ErrInfo err_info;
            if (ParseString(return_token, err_info))
            {
                data_.tokens_.push_back(std::move(return_token));
            }
            else
            {
                err_reporter_.AddError(std::move(err_info));
                while (!TokenIsOver())
                {
                    Advance();
                }
            }
        }
        break;
        case '-':
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9': {
            Token return_token;
            ErrInfo err_info;
            if (ParseNumber(return_token, err_info))
            {
                data_.tokens_.push_back(std::move(return_token));
            }
            else
            {
                err_reporter_.AddError(std::move(err_info));
                while (!TokenIsOver())
                {
                    Advance();
                }
            }
            break;
        }
        case 't':
        case 'f':
        case 'n': {
            Token return_token;
            ErrInfo err_info;
            if (ParseLiteral(return_token, err_info))
            {
                data_.tokens_.push_back(std::move(return_token));
            }
            else
            {
                err_reporter_.AddError(std::move(err_info));
                while (!TokenIsOver())
                {
                    Advance();
                }
            }
            break;
        }
        case '\n':
            // 额外添加分支处理换行符记录，这样不用在一开始将原始json字符串拆分，性能更好
            Advance();
            ++cur_row_;
            cur_col_ = 0;
            break;
        default:
            // windows下，如果不加是否是ascii字符的判断，并且读取到了json文件中的非ascii字符，此时使用isspace会导致程序崩溃
            if (IsAscii(data_.source_[cur_index_]) && std::isspace(data_.source_[cur_index_]) != 0)
            {
                Advance();
            }
            else
            {
                // 初始化返回参数
                const POS_T line_begin = data_.lines_index_[cur_row_].first;
                const POS_T line_end = data_.lines_index_[cur_row_].second;
                ErrInfo err_info = {ERR_UNKNOWN_VALUE, data_.source_.substr(line_begin, line_end - line_begin),
                                    cur_row_, cur_col_, 0};

                // 找到token结束位置
                LENGTH_T count = 0;
                while (!IsAscii(data_.source_[cur_index_]) || !TokenIsOver())
                {
                    Advance();
                    ++count;
                }

                err_info.len_ = count;
                err_reporter_.AddError(std::move(err_info));
            }
        }
    }

    // 最后读完字符串添加一个EOF
    data_.tokens_.push_back(MakeToken("", TokenType::EOF_));
}

bool Lexer::TokenIsOver() const noexcept
{
    const char cur_char = data_.source_[cur_index_];
    return std::isspace(cur_char) != 0 || cur_char == ']' || cur_char == '\0' || cur_char == '}' || cur_char == ',' ||
           cur_char == ':';
}

bool Lexer::IsAtEnd() const noexcept
{
    return cur_index_ == data_.source_.length();
}

bool Lexer::IsEndOfLine() const noexcept
{
    return data_.source_[cur_index_] == '\n';
}

char Lexer::Current() const noexcept
{
    return data_.source_[cur_index_];
}

char Lexer::Advance() noexcept
{
    if (!IsAtEnd())
    {
        ++cur_col_;
        return data_.source_[++cur_index_];
    }
    return '\0';
}

Token Lexer::MakeToken(std::string &&str, TokenType type) const noexcept
{
    const LENGTH_T token_len = str.length();
    return {std::move(str), type, cur_row_, cur_col_, token_len};
}

bool Lexer::ParseString(Token &return_token, ErrInfo &err_info)
{
    // 初始化返回参数
    return_token = {"", TokenType::STR, cur_row_, cur_col_, 0};
    const POS_T line_begin = data_.lines_index_[cur_row_].first;
    const POS_T line_end = data_.lines_index_[cur_row_].second;
    err_info = {"", data_.source_.substr(line_begin, line_end - line_begin), cur_row_, cur_col_, 0};

    StringDfaStat cur_stat = StringDfaStat::STRING_START;
    std::string unicode_buffer; // 暂时存储unicode转移序列

    // 用于错误高亮打印，因为字符串有许多转义序列，无法直接将return_token的长度等价于高亮长度，所以定义此变量用于高亮打印
    LENGTH_T err_highlight_len = 0;

    while (cur_stat != StringDfaStat::STRING_END && cur_stat != StringDfaStat::ERROR)
    {
        if (IsEndOfLine())
        {
            // 在非StringDfaStat::STRING_END情况下结束一行，意味着json字符串没有被引号括起来
            cur_stat = StringDfaStat::ERROR;
            err_info.err_desc_ = ERR_MISSING_QUOTATION_MARK;
            break;
        }

        const char cur_char = Current();

        switch (cur_stat)
        {
        case StringDfaStat::STRING_START:
            if (cur_char == '"')
            {
                cur_stat = StringDfaStat::IN_STRING;
            }
            else
            {
                cur_stat = StringDfaStat::ERROR;
                err_info.err_desc_ = ERR_MISSING_QUOTATION_MARK;
            }

            Advance();
            break;

        case StringDfaStat::IN_STRING:
            if (cur_char == '"')
            {
                cur_stat = StringDfaStat::STRING_END;
            }
            else if (cur_char == '\\')
            {
                cur_stat = StringDfaStat::STRING_ESCAPE;
            }
            else
            {
                return_token.raw_value_ += cur_char;
            }

            ++err_highlight_len;
            Advance();
            break;

        case StringDfaStat::STRING_ESCAPE:
            switch (cur_char)
            {
            case '\\':
                return_token.raw_value_ += '\\';
                cur_stat = StringDfaStat::IN_STRING;
                break;
            case '"':
                return_token.raw_value_ += '"';
                cur_stat = StringDfaStat::IN_STRING;
                break;
            case '/':
                return_token.raw_value_ += '/';
                cur_stat = StringDfaStat::IN_STRING;
                break;
            case 'b':
                return_token.raw_value_ += '\b';
                cur_stat = StringDfaStat::IN_STRING;
                break;
            case 'f':
                return_token.raw_value_ += '\f';
                cur_stat = StringDfaStat::IN_STRING;
                break;
            case 'r':
                return_token.raw_value_ += '\r';
                cur_stat = StringDfaStat::IN_STRING;
                break;
            case 'n':
                return_token.raw_value_ += '\n';
                cur_stat = StringDfaStat::IN_STRING;
                break;
            case 't':
                return_token.raw_value_ += '\t';
                cur_stat = StringDfaStat::IN_STRING;
                break;
            case 'u':
                cur_stat = StringDfaStat::STRING_UNICODE_START;
                unicode_buffer = "\\u";
                break;
            default:
                cur_stat = StringDfaStat::ERROR;
                err_info.err_desc_ = ERR_INVALID_ESCAPE;
                break;
            }

            err_highlight_len += 2; // 一个"\"转义序列长度，例如"\n"
            Advance();
            break;

        case StringDfaStat::STRING_UNICODE_START:
            for (int i = 0; i < 4; ++i)
            {
                if (IsEndOfLine())
                {
                    cur_stat = StringDfaStat::ERROR;
                    err_info.err_desc_ = ERR_INCOMPLETE_UNICODE_ESCAPE;
                    break;
                }

                if (const char cur = Current();
                    std::isdigit(cur) != 0 || (cur >= 'a' && cur <= 'f') || (cur >= 'A' && cur <= 'F'))
                {
                    unicode_buffer += cur;
                    Advance();
                }
                else
                {
                    cur_stat = StringDfaStat::ERROR;
                    err_info.err_desc_ = ERR_INVALID_UNICODE_ESCAPE;
                    break;
                }
            }
            if (cur_stat != StringDfaStat::ERROR)
            {
                return_token.raw_value_ += ConvertUnicodeEscape(unicode_buffer);
                cur_stat = StringDfaStat::IN_STRING;
            }

            err_highlight_len += 4; // 一个unicode序列长度，例如"4e00"
            break;
        }
    }

    return_token.len_ = return_token.raw_value_.length() + 2; // 这里的数字2为引号""本身长度
    err_info.len_ = err_highlight_len;

    return cur_stat == StringDfaStat::STRING_END;
}

bool Lexer::ParseNumber(Token &return_token, ErrInfo &err_info)
{
    // 初始化返回参数
    return_token = {"", TokenType::NUM, cur_row_, cur_col_, 0};
    const POS_T line_begin = data_.lines_index_[cur_row_].first;
    const POS_T line_end = data_.lines_index_[cur_row_].second;
    err_info = {"", data_.source_.substr(line_begin, line_end - line_begin), cur_row_, cur_col_, 0};

    NumberDfaStat cur_stat = NumberDfaStat::NUMBER_START;

    while (cur_stat != NumberDfaStat::NUMBER_END && cur_stat != NumberDfaStat::ERROR)
    {
        const char cur_char = Current();
        if (TokenIsOver())
        {
            if (cur_stat == NumberDfaStat::NUMBER_ZERO || cur_stat == NumberDfaStat::NUMBER_INTEGRAL ||
                cur_stat == NumberDfaStat::NUMBER_EXPONENT || cur_stat == NumberDfaStat::NUMBER_FRACTION)
            {
                cur_stat = NumberDfaStat::NUMBER_END;
            }
            else
            {
                err_info.err_desc_ = ERR_INCOMPLETE_NUMBER;
                cur_stat = NumberDfaStat::ERROR;
            }
            break;
        }

        switch (cur_stat)
        {
        case NumberDfaStat::NUMBER_START:
            if (cur_char == '0')
            {
                cur_stat = NumberDfaStat::NUMBER_ZERO;
            }
            else if (cur_char == '-')
            {
                cur_stat = NumberDfaStat::NUMBER_SIGN;
            }
            else if (std::isdigit(cur_char) != 0)
            {
                cur_stat = NumberDfaStat::NUMBER_INTEGRAL;
            }
            else
            {
                err_info.err_desc_ = ERR_INVALID_NUMBER;
                cur_stat = NumberDfaStat::ERROR;
            }
            break;

        case NumberDfaStat::NUMBER_ZERO:
            if (cur_char == '.')
            {
                cur_stat = NumberDfaStat::NUMBER_FRACTION_BEGIN;
            }
            else if (cur_char == 'e' || cur_char == 'E')
            {
                cur_stat = NumberDfaStat::NUMBER_EXPONENT_BEGIN;
            }
            else
            {
                err_info.err_desc_ = ERR_INVALID_NUMBER;
                cur_stat = NumberDfaStat::ERROR;
            }
            break;

        case NumberDfaStat::NUMBER_SIGN:
            if (cur_char == '0')
            {
                cur_stat = NumberDfaStat::NUMBER_ZERO;
            }
            else if (std::isdigit(cur_char) != 0)
            {
                cur_stat = NumberDfaStat::NUMBER_INTEGRAL;
            }
            else
            {
                err_info.err_desc_ = ERR_INVALID_NUMBER;
                cur_stat = NumberDfaStat::ERROR;
            }
            break;

        case NumberDfaStat::NUMBER_INTEGRAL:
            if (cur_char == '.')
            {
                cur_stat = NumberDfaStat::NUMBER_FRACTION_BEGIN;
            }
            else if (cur_char == 'e' || cur_char == 'E')
            {
                cur_stat = NumberDfaStat::NUMBER_EXPONENT_BEGIN;
            }
            else if (std::isdigit(cur_char) != 0)
            {
                cur_stat = NumberDfaStat::NUMBER_INTEGRAL;
            }
            else
            {
                err_info.err_desc_ = ERR_INVALID_NUMBER;
                cur_stat = NumberDfaStat::ERROR;
            }
            break;

        case NumberDfaStat::NUMBER_FRACTION_BEGIN:
            if (std::isdigit(cur_char) != 0)
            {
                cur_stat = NumberDfaStat::NUMBER_FRACTION;
            }
            else
            {
                err_info.err_desc_ = ERR_INVALID_NUMBER;
                cur_stat = NumberDfaStat::ERROR;
            }
            break;

        case NumberDfaStat::NUMBER_FRACTION:
            if (cur_char == 'e' || cur_char == 'E')
            {
                cur_stat = NumberDfaStat::NUMBER_EXPONENT_BEGIN;
            }
            else if (std::isdigit(cur_char) != 0)
            {
                cur_stat = NumberDfaStat::NUMBER_FRACTION;
            }
            else
            {
                err_info.err_desc_ = ERR_INVALID_NUMBER;
                cur_stat = NumberDfaStat::ERROR;
            }
            break;

        case NumberDfaStat::NUMBER_EXPONENT_BEGIN:
            if (cur_char == '-')
            {
                cur_stat = NumberDfaStat::NUMBER_EXPONENT_SIGN;
            }
            else if (std::isdigit(cur_char) != 0)
            {
                cur_stat = NumberDfaStat::NUMBER_EXPONENT;
            }
            else
            {
                err_info.err_desc_ = ERR_INVALID_NUMBER;
                cur_stat = NumberDfaStat::ERROR;
            }
            break;

        case NumberDfaStat::NUMBER_EXPONENT_SIGN:
            if (std::isdigit(cur_char) != 0)
            {
                cur_stat = NumberDfaStat::NUMBER_EXPONENT;
            }
            else
            {
                err_info.err_desc_ = ERR_INVALID_NUMBER;
                cur_stat = NumberDfaStat::ERROR;
            }
            break;

        case NumberDfaStat::NUMBER_EXPONENT:
            if (std::isdigit(cur_char) == 0)
            {
                err_info.err_desc_ = ERR_INVALID_NUMBER;
                cur_stat = NumberDfaStat::ERROR;
            }
            break;
        }
        return_token.raw_value_ += cur_char;
        Advance();
    }

    const POS_T token_len = return_token.raw_value_.length();
    return_token.len_ = token_len;
    err_info.len_ = token_len;

    return cur_stat == NumberDfaStat::NUMBER_END;
}

bool Lexer::ParseLiteral(Token &return_token, ErrInfo &err_info)
{
    // 初始化返回参数
    return_token = {"", TokenType::TRUE, cur_row_, cur_col_, 0};
    const POS_T line_begin = data_.lines_index_[cur_row_].first;
    const POS_T line_end = data_.lines_index_[cur_row_].second;
    err_info = {"", data_.source_.substr(line_begin, line_end - line_begin), cur_row_, cur_col_, 0};

    LiteralDfaStat cur_stat = LiteralDfaStat::LITERAL_START;

    while (cur_stat != LiteralDfaStat::LITERAL_END && cur_stat != LiteralDfaStat::ERROR)
    {
        // 已经到达行结尾，但是字面量没有到最后一个字符，则认为字面量不完整
        if (IsEndOfLine() && cur_stat != LiteralDfaStat::TRUE_E && cur_stat != LiteralDfaStat::FALSE_E &&
            cur_stat != LiteralDfaStat::NULL_L2)
        {
            cur_stat = LiteralDfaStat::ERROR;
            err_info.err_desc_ = ERR_INVALID_LITERAL;
            break;
        }

        const char cur_char = Current();

        switch (cur_stat)
        {
        case LiteralDfaStat::LITERAL_START:
            if (cur_char == 't')
            {
                return_token.type_ = TokenType::TRUE;
                cur_stat = LiteralDfaStat::TRUE_T;
            }
            else if (cur_char == 'f')
            {
                return_token.type_ = TokenType::FALSE;
                cur_stat = LiteralDfaStat::FALSE_F;
            }
            else if (cur_char == 'n')
            {
                return_token.type_ = TokenType::NULL_;
                cur_stat = LiteralDfaStat::NULL_N;
            }
            else
            {
                err_info.err_desc_ = ERR_INVALID_LITERAL;
                cur_stat = LiteralDfaStat::ERROR;
            }

            return_token.raw_value_ += cur_char;
            Advance();
            break;

        case LiteralDfaStat::TRUE_T:
            if (cur_char == 'r')
            {
                cur_stat = LiteralDfaStat::TRUE_R;
            }
            else
            {
                err_info.err_desc_ = ERR_INVALID_LITERAL + std::string(LITERAL_GUESS_TRUE);
                cur_stat = LiteralDfaStat::ERROR;
            }

            return_token.raw_value_ += cur_char;
            Advance();
            break;

        case LiteralDfaStat::TRUE_R:
            if (cur_char == 'u')
            {
                cur_stat = LiteralDfaStat::TRUE_U;
            }
            else
            {
                err_info.err_desc_ = ERR_INVALID_LITERAL + std::string(LITERAL_GUESS_TRUE);
                cur_stat = LiteralDfaStat::ERROR;
            }

            return_token.raw_value_ += cur_char;
            Advance();
            break;

        case LiteralDfaStat::TRUE_U:
            if (cur_char == 'e')
            {
                cur_stat = LiteralDfaStat::TRUE_E;
            }
            else
            {
                err_info.err_desc_ = ERR_INVALID_LITERAL + std::string(LITERAL_GUESS_TRUE);
                cur_stat = LiteralDfaStat::ERROR;
            }

            return_token.raw_value_ += cur_char;
            Advance();
            break;

        case LiteralDfaStat::TRUE_E:
            if (TokenIsOver())
            {
                cur_stat = LiteralDfaStat::LITERAL_END;
            }
            else
            {
                err_info.err_desc_ = ERR_INVALID_LITERAL + std::string(LITERAL_GUESS_TRUE);
                cur_stat = LiteralDfaStat::ERROR;
            }
            break;

        case LiteralDfaStat::FALSE_F:
            if (cur_char == 'a')
            {
                cur_stat = LiteralDfaStat::FALSE_A;
            }
            else
            {
                err_info.err_desc_ = ERR_INVALID_LITERAL + std::string(LITERAL_GUESS_FALSE);
                cur_stat = LiteralDfaStat::ERROR;
            }

            return_token.raw_value_ += cur_char;
            Advance();
            break;

        case LiteralDfaStat::FALSE_A:
            if (cur_char == 'l')
            {
                cur_stat = LiteralDfaStat::FALSE_L;
            }
            else
            {
                err_info.err_desc_ = ERR_INVALID_LITERAL + std::string(LITERAL_GUESS_FALSE);
                cur_stat = LiteralDfaStat::ERROR;
            }

            return_token.raw_value_ += cur_char;
            Advance();
            break;

        case LiteralDfaStat::FALSE_L:
            if (cur_char == 's')
            {
                cur_stat = LiteralDfaStat::FALSE_S;
            }
            else
            {
                err_info.err_desc_ = ERR_INVALID_LITERAL + std::string(LITERAL_GUESS_FALSE);
                cur_stat = LiteralDfaStat::ERROR;
            }

            return_token.raw_value_ += cur_char;
            Advance();
            break;

        case LiteralDfaStat::FALSE_S:
            if (cur_char == 'e')
            {
                cur_stat = LiteralDfaStat::FALSE_E;
            }
            else
            {
                err_info.err_desc_ = ERR_INVALID_LITERAL + std::string(LITERAL_GUESS_FALSE);
                cur_stat = LiteralDfaStat::ERROR;
            }

            return_token.raw_value_ += cur_char;
            Advance();
            break;

        case LiteralDfaStat::FALSE_E:
            if (TokenIsOver())
            {
                cur_stat = LiteralDfaStat::LITERAL_END;
            }
            else
            {
                err_info.err_desc_ = ERR_INVALID_LITERAL + std::string(LITERAL_GUESS_FALSE);
                cur_stat = LiteralDfaStat::ERROR;
            }
            break;

        case LiteralDfaStat::NULL_N:
            if (cur_char == 'u')
            {
                cur_stat = LiteralDfaStat::NULL_U;
            }
            else
            {
                err_info.err_desc_ = ERR_INVALID_LITERAL + std::string(LITERAL_GUESS_NULL);
                cur_stat = LiteralDfaStat::ERROR;
            }

            return_token.raw_value_ += cur_char;
            Advance();
            break;

        case LiteralDfaStat::NULL_U:
            if (cur_char == 'l')
            {
                cur_stat = LiteralDfaStat::NULL_L1;
            }
            else
            {
                err_info.err_desc_ = ERR_INVALID_LITERAL + std::string(LITERAL_GUESS_NULL);
                cur_stat = LiteralDfaStat::ERROR;
            }

            return_token.raw_value_ += cur_char;
            Advance();
            break;

        case LiteralDfaStat::NULL_L1:
            if (cur_char == 'l')
            {
                cur_stat = LiteralDfaStat::NULL_L2;
            }
            else
            {
                err_info.err_desc_ = ERR_INVALID_LITERAL + std::string(LITERAL_GUESS_NULL);
                cur_stat = LiteralDfaStat::ERROR;
            }

            return_token.raw_value_ += cur_char;
            Advance();
            break;

        case LiteralDfaStat::NULL_L2:
            if (TokenIsOver())
            {
                cur_stat = LiteralDfaStat::LITERAL_END;
            }
            else
            {
                err_info.err_desc_ = ERR_INVALID_LITERAL + std::string(LITERAL_GUESS_NULL);
                cur_stat = LiteralDfaStat::ERROR;
            }
            break;
        }
    }

    const POS_T token_len = return_token.raw_value_.length();
    return_token.len_ = token_len;
    err_info.len_ = token_len;

    return cur_stat == LiteralDfaStat::LITERAL_END;
}

void Parser::Parse() noexcept
{
    // 因为json顶层必须是对象或者数据，所以第一个json token肯定是"{"或者"]"
    if (const Token *cur_token = Current(); cur_token->type_ == TokenType::LBRACE)
    {
        json_ = ParseObject();
    }
    else if (cur_token->type_ == TokenType::LBRACKET)
    {
        json_ = ParseArray();
    }
    else
    {
        // 顶层不是对象或者数组，构建错误信息，抛异常
        MakeErrInfo(ERR_MISMATCH_TOP_LEVEL, cur_token);
    }

} // namespace simple_json

bool Parser::ParseValue(JsonValue &return_value) noexcept
{
    switch (const Token *cur_token = Current(); cur_token->type_)
    {
    case TokenType::LBRACE:
        return_value = ParseObject();
        return true;
    case TokenType::LBRACKET:
        return_value = ParseArray();
        return true;
    case TokenType::STR:
        return_value = {cur_token->raw_value_};
        return true;
    case TokenType::NUM:
        return_value = ParseNumber();
        return true;
    case TokenType::TRUE:
        return_value = {true};
        return true;
    case TokenType::FALSE:
        return_value = {false};
        return true;
    case TokenType::NULL_:
        return_value = {nullptr};
        return true;
    default:
        return false;
    }
}

JsonValue Parser::ParseObject() noexcept
{
    std::unordered_map<std::string, JsonValue> ret_object;
    if (!Consume(Current(), TokenType::LBRACE)) // need fix
    {
        MakeErrInfo(ERR_TYPE_NOT_OBJECT, Current());
        return {ret_object};
    }
    Advance();

    while (Current()->type_ != TokenType::RBRACE && Current()->type_ != TokenType::EOF_)
    {
        std::string key;
        // json对象的键必须为字符串
        if (!Consume(Current(), TokenType::STR)) // need fix
        {
            MakeErrInfo(ERR_OBJECT_KEY_MUST_BE_STRING, Current());

            Synchronize();
            if (const Token *cur_token = Current(); cur_token->type_ == TokenType::COMMA)
            {
                Advance();
                if (!ALLOW_TRAILING_COMMA && Current()->type_ == TokenType::RBRACE)
                {
                    MakeErrInfo(ERR_TRAILING_COMMA, Prev());
                }
            }
            continue;
        }
        key = Current()->raw_value_;
        Advance();

        // 字符串键后必须跟冒号
        if (!Consume(Current(), TokenType::COLON)) // need fix
        {
            MakeErrInfo(ERR_COLON_EXPECTED, Current());

            Synchronize();
            if (const Token *cur_token = Current(); cur_token->type_ == TokenType::COMMA)
            {
                Advance();
                if (!ALLOW_TRAILING_COMMA && Current()->type_ == TokenType::RBRACE)
                {
                    MakeErrInfo(ERR_TRAILING_COMMA, Prev());
                }
            }
            continue;
        }
        Advance();

        JsonValue value; // 键对应的值
        if (!ParseValue(value))
        {
            MakeErrInfo(ERR_EXPECTED_JSON_VALUE_TYPE, Current());

            Synchronize();
            if (const Token *cur_token = Current(); cur_token->type_ == TokenType::COMMA)
            {
                Advance();
                if (!ALLOW_TRAILING_COMMA && Current()->type_ == TokenType::RBRACE)
                {
                    MakeErrInfo(ERR_TRAILING_COMMA, Prev());
                }
            }
            continue;
        }
        ret_object[key] = std::move(value);
        Advance();

        // 遇到"}"，对象结束，跳出循环
        if (Current()->type_ == TokenType::RBRACE)
        {
            break;
        }

        if (!Consume(Current(), TokenType::COMMA))
        {
            // 如果当前不是逗号，那么就语法有错误，进入恐慌模式
            MakeErrInfo(ERR_COMMA_OR_BRACE_EXPECTED, Current());

            Synchronize();
            if (const Token *cur_token = Current(); cur_token->type_ == TokenType::COMMA)
            {
                Advance();
                if (!ALLOW_TRAILING_COMMA && Current()->type_ == TokenType::RBRACE)
                {
                    MakeErrInfo(ERR_TRAILING_COMMA, Prev());
                }
            }
            continue;
        }

        // 如果当前是逗号，那么检查是否为尾随逗号
        Advance();
        if (!ALLOW_TRAILING_COMMA && Current()->type_ == TokenType::RBRACE)
        {
            MakeErrInfo(ERR_TRAILING_COMMA, Prev());
        }
    }

    if (Current()->type_ == TokenType::EOF_)
    {
        // 如果当前为EOF_，则认为数组没有关闭
        const Token *prev_token = &json_data_.tokens_[cur_token_index_ - 1]; // EOF_的前一个token
        MakeErrInfo(ERR_OBJECT_NOT_CLOSED, prev_token);
    }

    return {ret_object};
}

JsonValue Parser::ParseArray() noexcept
{
    std::vector<JsonValue> ret_array;
    if (!Consume(Current(), TokenType::LBRACKET))
    {
        MakeErrInfo(ERR_TYPE_NOT_ARRAY, Current());
        return {ret_array};
    }
    Advance();

    while (Current()->type_ != TokenType::RBRACKET && Current()->type_ != TokenType::EOF_)
    {
        JsonValue return_value;
        if (!ParseValue(return_value))
        {
            MakeErrInfo(ERR_EXPECTED_JSON_VALUE_TYPE, Current());

            Synchronize();
            if (const Token *cur_token = Current(); cur_token->type_ == TokenType::COMMA)
            {
                Advance();
                if (!ALLOW_TRAILING_COMMA && Current()->type_ == TokenType::RBRACKET)
                {
                    MakeErrInfo(ERR_TRAILING_COMMA, Prev());
                }
            }
            continue;
        }
        ret_array.push_back(std::move(return_value));

        // 判断下一个token是是否是"]"，如果是，则认为数组结束
        if (Peek()->type_ == TokenType::RBRACKET)
        {
            Advance();
            break;
        }

        // 不是"]"，数组未结束，那么认定下一个token为","，否则视为语法错误
        if (!Consume(Peek(), TokenType::COMMA))
        {
            // 抛出语法错误
            // 进入恐慌模式前的当前token
            const Token *before_panic = Current();
            // 第三个参数定位了高亮的起始位置，因为这里期望有一个逗号,
            // 所以不能直接在token的起始位置进行高亮，必须以该token长度为基准进行偏移
            MakeErrInfo(ERR_COMMA_OR_BRACKET_EXPECTED, before_panic, before_panic->col_ + before_panic->len_, 1);
            // 进入恐慌模式
            Synchronize();

            // 判断下一个token是否为"]"，如果是，并且不允许尾随逗号，那么认为此处为语法错误
            if (const Token *after_panic = Current(); after_panic->type_ == TokenType::COMMA)
            {
                // 是尾随逗号，报错
                if (!ALLOW_TRAILING_COMMA && Peek()->type_ == TokenType::RBRACKET)
                {
                    MakeErrInfo(ERR_TRAILING_COMMA, Current());
                    Advance();
                    break;
                }

                // 不是尾随逗号，跳过逗号，从头开始解析
                Advance();
            }
            continue;
        }
        Advance();

        // 此时Current()->type_应该为TokenType::COMMA
        // 直接判断逗号是不是尾随逗号
        if (!ALLOW_TRAILING_COMMA && Peek()->type_ == TokenType::RBRACKET)
        {
            MakeErrInfo(ERR_TRAILING_COMMA, Current());
        }
        // 跳过当前逗号token
        Advance();
    }

    if (Current()->type_ == TokenType::EOF_)
    {
        // 如果当前为EOF_，则认为数组没有关闭
        const Token *last_token = &json_data_.tokens_[cur_token_index_ - 1]; // EOF_的前一个token
        MakeErrInfo(ERR_ARRAY_NOT_CLOSED, last_token, last_token->col_ + last_token->len_, 1);
    }

    return {ret_array};
}

JsonValue Parser::ParseNumber() noexcept
{
    const Token *cur_token = Current();
    if (cur_token->raw_value_.find('e') != std::string::npos || cur_token->raw_value_.find('.') != std::string::npos)
    {
        return {std::stold(cur_token->raw_value_)};
    }

    return {std::stoll(cur_token->raw_value_)};
}

const Token *Parser::Prev() const noexcept
{
    if (cur_token_index_ - 1 >= 0)
    {
        return &json_data_.tokens_[cur_token_index_ - 1];
    }
    return nullptr;
}

const Token *Parser::Current() const noexcept
{
    if (cur_token_index_ < json_data_.tokens_.size())
    {
        return &json_data_.tokens_[cur_token_index_];
    }
    return nullptr;
}

const Token *Parser::Peek() const noexcept
{
    if (cur_token_index_ + 1 < json_data_.tokens_.size())
    {
        return &json_data_.tokens_[cur_token_index_ + 1];
    }
    return nullptr;
}

Token *Parser::Advance() noexcept
{
    if (cur_token_index_ + 1 < json_data_.tokens_.size())
    {
        return &json_data_.tokens_[++cur_token_index_];
    }
    return nullptr;
}

bool Parser::Consume(const Token *token, TokenType token_type) noexcept
{
    // 我们这里的判断主要应对Consume(Peek())中的Peek()返回nullptr的情况
    // 目前只是demo代码，所以简单处理一下
    if (token != nullptr)
    {
        if (token->type_ == token_type)
        {
            return true;
        }
    }
    return false;
}

void Parser::MakeErrInfo(std::string err_desc, const Token *cur_token, size_t highlight_pos,
                         size_t highlight_len) noexcept
{
    // 判断是否未指定后两个参数
    highlight_len = highlight_len == 0 ? cur_token->len_ : highlight_len;
    highlight_pos = highlight_pos == 0 ? cur_token->col_ : highlight_pos;

    const POS_T line_begin = json_data_.lines_index_[cur_token->row_].first;
    const POS_T line_end = json_data_.lines_index_[cur_token->row_].second;
    ErrInfo err_info{std::move(err_desc), json_data_.source_.substr(line_begin, line_end - line_begin), cur_token->row_,
                     highlight_pos, highlight_len};
    err_reporter_.AddError(std::move(err_info));
}

void Parser::Synchronize() noexcept
{
    Advance();
    while (Current()->type_ != TokenType::EOF_)
    {
        switch (const Token *cur_token = Current(); cur_token->type_)
        {
        case TokenType::COMMA:
        case TokenType::LBRACE:
        case TokenType::LBRACKET:
        case TokenType::RBRACE:
        case TokenType::RBRACKET:
        case TokenType::EOF_:
            return;
        default:
            Advance(); // 未找到安全值则一致消耗token
        }
    }
}

} // namespace simple_json
