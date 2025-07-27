#include "lexer_parser.h"
#include "config.h"
#include "json_type.h"
#include "utilities.h"

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
            data_.tokens_.push_back(MakeToken("{", TokenType::RBRACE));
            Advance();
            break;
        case '[':
            data_.tokens_.push_back(MakeToken("[", TokenType::LBRACKET));
            Advance();
            break;
        case ']':
            data_.tokens_.push_back(MakeToken("[", TokenType::RBRACKET));
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
            ++cur_row_;
            cur_col_ = 0;
            Advance();
            break;
        default:
            if (std::isspace(data_.source_[cur_index_]) != 0)
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
                while (!TokenIsOver())
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
    return std::isspace(cur_char) != 0 || cur_char == ']' || cur_char == '\0' || cur_char == '}' || cur_char == ',';
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
    return_token = {"", TokenType::STR, cur_row_, cur_row_, 0};
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

    return_token.len_ = return_token.raw_value_.length();
    err_info.len_ = err_highlight_len;

    return cur_stat == StringDfaStat::STRING_END;
}

bool Lexer::ParseNumber(Token &return_token, ErrInfo &err_info)
{
    // 初始化返回参数
    return_token = {"", TokenType::NUM, cur_row_, cur_row_, 0};
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
    return_token = {"", TokenType::TRUE, cur_row_, cur_row_, 0};
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
    if (const Token *cur_token = Current(); cur_token->type_ == TokenType::LBRACKET)
    {
        json_ = ParseObject();
    }
    else if (cur_token->type_ == TokenType::LBRACKET)
    {
        json_ = ParseArray();
    }
    else
    {
        // 顶层不是对象或者数组，构建错误信息，准备抛异常
        const POS_T line_begin = json_data_.lines_index_[cur_token->row_].first;
        const POS_T line_end = json_data_.lines_index_[cur_token->row_].second;
        ErrInfo err_info{ERR_MISMATCH_TOP_LEVEL, json_data_.source_.substr(line_begin, line_end - line_begin),
                         cur_token->row_, cur_token->col_, cur_token->len_};
        err_reporter_.AddError(std::move(err_info));
    }

} // namespace simple_json

JsonValue Parser::ParseValue() noexcept
{
    switch (const Token *cur_token = Current(); cur_token->type_)
    {
    }
}

JsonValue Parser::ParseObject() noexcept
{
}

JsonValue Parser::ParseArray() noexcept
{
}

const Token *Parser::Current() const noexcept
{
    if (cur_token_index_ < json_data_.tokens_.size())
    {
        return &json_data_.tokens_[cur_token_index_];
    }
    return nullptr;
}

const Token *Parser::Advance() noexcept
{
    if (cur_token_index_ + 1 < json_data_.tokens_.size())
    {
        return &json_data_.tokens_[++cur_token_index_];
    }
    return nullptr;
}

bool Parser::Consume(TokenType token_type, std::string err_desc) noexcept
{
    if (const Token *cur_token = Current(); cur_token->type_ != token_type)
    {
        const POS_T line_begin = json_data_.lines_index_[cur_token->row_].first;
        const POS_T line_end = json_data_.lines_index_[cur_token->row_].second;
        ErrInfo err_info{std::move(err_desc), json_data_.source_.substr(line_begin, line_end - line_begin),
                         cur_token->row_, cur_token->col_, cur_token->len_};
        err_reporter_.AddError(std::move(err_info));
        return false;
    }
    return true;
}

} // namespace simple_json
