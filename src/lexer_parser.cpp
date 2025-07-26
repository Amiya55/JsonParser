#include "lexer_parser.h"
#include "config.h"
#include "utilities.h"
#include <stdexcept>
#include <string>

namespace simpleJson
{
bool ErrReporter::hasError() const noexcept
{
    return !_errors.empty();
}

void ErrReporter::throwError(const bool throwAll) const
{
    if (!hasError())
        return;

    std::string errorPrintInfo;
    const size_t errorCount = throwAll ? _errors.size() : 1;
    for (size_t i = 0; i < errorCount; i++)
    {
        const POS_T errRow = _errors[i].row + 1; // 错误所在行的行号，面对用户，索引从1开始，所以加1
        const POS_T errCol = _errors[i].col;

        // 构建错误所在行信息
        std::string errInfo("[Row: " + std::to_string(errRow) + ", Col: " + std::to_string(errCol) + "] " +
                            _errors[i].errDesc + "\n");
        std::string LineInfo(std::to_string(errRow) + " | " + _errors[i].currentLine);
        errorPrintInfo.append(errInfo);
        errorPrintInfo.append(LineInfo);

        // 构建空格缩进
        // 下面的errLineIndex是行号长度，数字3为字符串" | "长度，
        // _messages[i].errToken.col为本行距离错误token首字符的长度，数字1为索引差值
        std::string indent(std::to_string(errRow).length() + 3 + _errors[i].col - 1, ' ');
        // 高亮错误token
        std::string highlight(_errors[i].len, '~');
        errorPrintInfo.append(indent + highlight);

        // 构建分隔符
        errorPrintInfo.append("\n- - - - - - - - - - -\n");
    }

    throw std::runtime_error(errorPrintInfo);
}

void Lexer::_splitLines() noexcept
{
    POS_T begin = 0;
    POS_T curLineIndex = 0;
    for (POS_T end = 0; end < _data.source.length(); ++end)
    {
        if (_data.source[end] == '\n' || end == _data.source.length() - 1)
        {
            _data.linesIndex[curLineIndex] = std::make_pair(begin, end + 1);
            begin = end + 1;
            ++curLineIndex;
        }
    }
}

void Lexer::_scan()
{
    for (_curIndex = 0; _curIndex < _data.source.length();)
    {
        switch (_data.source[_curIndex])
        {
        case '{':
            _data.tokens.push_back(_makeToken("{", TokenType::LBRACE, _curRow, _curCol));
            _advance();
            break;
        case '}':
            _data.tokens.push_back(_makeToken("{", TokenType::RBRACE, _curRow, _curCol));
            _advance();
            break;
        case '[':
            _data.tokens.push_back(_makeToken("[", TokenType::LBRACKET, _curRow, _curCol));
            _advance();
            break;
        case ']':
            _data.tokens.push_back(_makeToken("[", TokenType::RBRACKET, _curRow, _curCol));
            _advance();
            break;
        case ',':
            _data.tokens.push_back(_makeToken(",", TokenType::COMMA, _curRow, _curCol));
            _advance();
            break;
        case ':':
            _data.tokens.push_back(_makeToken(":", TokenType::COLON, _curRow, _curCol));
            _advance();
            break;
        case '\"': {
            Token returnToken;
            ErrInfo errInfo;
            if (_parseString(returnToken, errInfo))
            {
                _data.tokens.push_back(std::move(returnToken));
            }
            else
            {
                _errReporter.addError(std::move(errInfo));
                while (!_tokenIsOver())
                    _advance();
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
            Token returnToken;
            ErrInfo errInfo;
            if (_parseNumber(returnToken, errInfo))
            {
                _data.tokens.push_back(std::move(returnToken));
            }
            else
            {
                _errReporter.addError(std::move(errInfo));
                while (!_tokenIsOver())
                    _advance();
            }
            break;
        }
        case 't':
        case 'f':
        case 'n': {
            Token returnToken;
            ErrInfo errInfo;
            if (_parseLiteral(returnToken, errInfo))
            {
                _data.tokens.push_back(std::move(returnToken));
            }
            else
            {
                _errReporter.addError(std::move(errInfo));
                while (!_tokenIsOver())
                    _advance();
            }
            break;
        }
        case '\n':
            // 额外添加分支处理换行符记录，这样不用在一开始将原始json字符串拆分，性能更好
            ++_curRow;
            _curCol = 0;
            _advance();
            break;
        default:
            if (std::isspace(_data.source[_curIndex]))
            {
                _advance();
            }
            else
            {
                // 初始化返回参数
                const POS_T lineBegin = _data.linesIndex[_curRow].first;
                const POS_T lineEnd = _data.linesIndex[_curRow].second;
                ErrInfo errInfo = {ERR_UNKNOWN_VALUE, _data.source.substr(lineBegin, lineEnd - lineBegin), _curRow,
                                   _curCol, 0};

                // 找到token结束位置
                LENGTH_T count = 0;
                while (!_tokenIsOver())
                {
                    _advance();
                    ++count;
                }

                errInfo.len = count;
                _errReporter.addError(std::move(errInfo));
            }
        }
    }
}

bool Lexer::_tokenIsOver() const noexcept
{
    const char curChar = _data.source[_curIndex];
    return std::isspace(curChar) || curChar == ']' || curChar == '\0' || curChar == '}' || curChar == ',';
}

bool Lexer::_isAtEnd() const noexcept
{
    return _curIndex == _data.source.length();
}

bool Lexer::_isEndOfLine() const noexcept
{
    return _data.source[_curIndex] == '\n';
}

char Lexer::_current() const noexcept
{
    return _data.source[_curIndex];
}

char Lexer::_peek() const noexcept
{
    if (!_isAtEnd())
    {
        return _data.source[_curIndex + 1];
    }
    return '\0';
}

char Lexer::_advance() noexcept
{
    if (!_isAtEnd())
    {
        ++_curCol;
        return _data.source[++_curIndex];
    }
    return '\0';
}

Token Lexer::_makeToken(std::string &&str, TokenType type, POS_T row, POS_T col) noexcept
{
    const LENGTH_T tokenLen = str.length();
    return {std::move(str), type, row, col, tokenLen};
}

bool Lexer::_parseString(Token &returnToken, ErrInfo &errInfo)
{
    // 初始化返回参数
    returnToken = {"", TokenType::STR, _curRow, _curRow, 0};
    const POS_T lineBegin = _data.linesIndex[_curRow].first;
    const POS_T lineEnd = _data.linesIndex[_curRow].second;
    errInfo = {"", _data.source.substr(lineBegin, lineEnd - lineBegin), _curRow, _curCol, 0};

    StringDfaStat curStat = StringDfaStat::STRING_START;
    std::string unicode_buffer; // 暂时存储unicode转移序列

    while (curStat != StringDfaStat::STRING_END && curStat != StringDfaStat::ERROR)
    {
        if (_isEndOfLine())
        {
            // 在非StringDfaStat::STRING_END情况下结束一行，意味着json字符串没有被引号括起来
            curStat = StringDfaStat::ERROR;
            errInfo.errDesc = ERR_MISSING_QUOTATION_MARK;
            break;
        }

        const char curChar = _current();

        switch (curStat)
        {
        case StringDfaStat::STRING_START:
            if (curChar == '"')
            {
                curStat = StringDfaStat::IN_STRING;
            }
            else
            {
                curStat = StringDfaStat::ERROR;
                errInfo.errDesc = ERR_MISSING_QUOTATION_MARK;
            }
            _advance();
            break;

        case StringDfaStat::IN_STRING:
            if (curChar == '"')
            {
                curStat = StringDfaStat::STRING_END;
            }
            else if (curChar == '\\')
            {
                curStat = StringDfaStat::STRING_ESCAPE;
            }
            else
            {
                returnToken.rawValue += curChar;
            }
            _advance();
            break;

        case StringDfaStat::STRING_ESCAPE:
            switch (curChar)
            {
            case '\\':
                returnToken.rawValue += '\\';
                curStat = StringDfaStat::IN_STRING;
                break;
            case '"':
                returnToken.rawValue += '"';
                curStat = StringDfaStat::IN_STRING;
                break;
            case '/':
                returnToken.rawValue += '/';
                curStat = StringDfaStat::IN_STRING;
                break;
            case 'b':
                returnToken.rawValue += '\b';
                curStat = StringDfaStat::IN_STRING;
                break;
            case 'f':
                returnToken.rawValue += '\f';
                curStat = StringDfaStat::IN_STRING;
                break;
            case 'r':
                returnToken.rawValue += '\r';
                curStat = StringDfaStat::IN_STRING;
                break;
            case 'n':
                returnToken.rawValue += '\n';
                curStat = StringDfaStat::IN_STRING;
                break;
            case 't':
                returnToken.rawValue += '\t';
                curStat = StringDfaStat::IN_STRING;
                break;
            case 'u':
                curStat = StringDfaStat::STRING_UNICODE_START;
                unicode_buffer = "\\u";
                break;
            default:
                curStat = StringDfaStat::ERROR;
                errInfo.errDesc = ERR_INVALID_ESCAPE;
                break;
            }
            _advance();
            break;

        case StringDfaStat::STRING_UNICODE_START:
            for (int i = 0; i < 4; ++i)
            {
                if (_isEndOfLine())
                {
                    curStat = StringDfaStat::ERROR;
                    errInfo.errDesc = ERR_MISSING_QUOTATION_MARK;
                    break;
                }

                if (std::isdigit(_current()) || (_current() >= 'a' && _current() <= 'f') ||
                    (_current() >= 'A' && _current() <= 'F'))
                {
                    unicode_buffer += _current();
                    _advance();
                }
                else
                {
                    curStat = StringDfaStat::ERROR;
                    errInfo.errDesc = ERR_INVALID_ESCAPE;
                    break;
                }
            }
            if (curStat != StringDfaStat::ERROR)
            {
                returnToken.rawValue += convert_unicode_escape(unicode_buffer);
                curStat = StringDfaStat::IN_STRING;
            }
            break;
        }
    }

    POS_T tokenLen = returnToken.rawValue.length();
    returnToken.len = tokenLen;
    errInfo.len = tokenLen;

    return curStat == StringDfaStat::STRING_END;
}

bool Lexer::_parseNumber(Token &returnToken, ErrInfo &errInfo)
{
    // 初始化返回参数
    returnToken = {"", TokenType::NUM, _curRow, _curRow, 0};
    const POS_T lineBegin = _data.linesIndex[_curRow].first;
    const POS_T lineEnd = _data.linesIndex[_curRow].second;
    errInfo = {"", _data.source.substr(lineBegin, lineEnd - lineBegin), _curRow, _curCol, 0};

    NumberDfaStat curStat = NumberDfaStat::NUMBER_START;

    while (curStat != NumberDfaStat::NUMBER_END && curStat != NumberDfaStat::ERROR)
    {
        const char curChar = _current();
        if (_tokenIsOver())
        {
            if (curStat == NumberDfaStat::NUMBER_ZERO || curStat == NumberDfaStat::NUMBER_INTEGRAL ||
                curStat == NumberDfaStat::NUMBER_EXPONENT || curStat == NumberDfaStat::NUMBER_FRACTION)
            {
                curStat = NumberDfaStat::NUMBER_END;
            }
            else
            {
                errInfo.errDesc = ERR_INCOMPLETE_NUMBER;
                curStat = NumberDfaStat::ERROR;
            }
            break;
        }

        switch (curStat)
        {
        case NumberDfaStat::NUMBER_START:
            if (curChar == '0')
            {
                curStat = NumberDfaStat::NUMBER_ZERO;
            }
            else if (curChar == '-')
            {
                curStat = NumberDfaStat::NUMBER_SIGN;
            }
            else if (std::isdigit(curChar))
            {
                curStat = NumberDfaStat::NUMBER_INTEGRAL;
            }
            else
            {
                errInfo.errDesc = ERR_INVALID_NUMBER;
                curStat = NumberDfaStat::ERROR;
            }
            break;

        case NumberDfaStat::NUMBER_ZERO:
            if (curChar == '.')
            {
                curStat = NumberDfaStat::NUMBER_FRACTION_BEGIN;
            }
            else if (curChar == 'e' || curChar == 'E')
            {
                curStat = NumberDfaStat::NUMBER_EXPONENT_BEGIN;
            }
            else
            {
                errInfo.errDesc = ERR_INVALID_NUMBER;
                curStat = NumberDfaStat::ERROR;
            }
            break;

        case NumberDfaStat::NUMBER_SIGN:
            if (std::isdigit(curChar))
            {
                curStat = NumberDfaStat::NUMBER_INTEGRAL;
            }
            else
            {
                errInfo.errDesc = ERR_INVALID_NUMBER;
                curStat = NumberDfaStat::ERROR;
            }
            break;

        case NumberDfaStat::NUMBER_INTEGRAL:
            if (curChar == '.')
            {
                curStat = NumberDfaStat::NUMBER_FRACTION_BEGIN;
            }
            else if (curChar == 'e' || curChar == 'E')
            {
                curStat = NumberDfaStat::NUMBER_EXPONENT_BEGIN;
            }
            else if (std::isdigit(curChar))
            {
                curStat = NumberDfaStat::NUMBER_INTEGRAL;
            }
            else
            {
                errInfo.errDesc = ERR_INVALID_NUMBER;
                curStat = NumberDfaStat::ERROR;
            }
            break;

        case NumberDfaStat::NUMBER_FRACTION_BEGIN:
            if (std::isdigit(curChar))
            {
                curStat = NumberDfaStat::NUMBER_FRACTION;
            }
            else
            {
                errInfo.errDesc = ERR_INVALID_NUMBER;
                curStat = NumberDfaStat::ERROR;
            }
            break;

        case NumberDfaStat::NUMBER_FRACTION:
            if (curChar == 'e' || curChar == 'E')
            {
                curStat = NumberDfaStat::NUMBER_EXPONENT_BEGIN;
            }
            else if (std::isdigit(curChar))
            {
                curStat = NumberDfaStat::NUMBER_FRACTION;
            }
            else
            {
                errInfo.errDesc = ERR_INVALID_NUMBER;
                curStat = NumberDfaStat::ERROR;
            }
            break;

        case NumberDfaStat::NUMBER_EXPONENT_BEGIN:
            if (curChar == '-')
            {
                curStat = NumberDfaStat::NUMBER_EXPONENT_SIGN;
            }
            else if (std::isdigit(curChar))
            {
                curStat = NumberDfaStat::NUMBER_EXPONENT;
            }
            else
            {
                errInfo.errDesc = ERR_INVALID_NUMBER;
                curStat = NumberDfaStat::ERROR;
            }
            break;

        case NumberDfaStat::NUMBER_EXPONENT_SIGN:
            if (std::isdigit(curChar))
            {
                curStat = NumberDfaStat::NUMBER_EXPONENT;
            }
            else
            {
                errInfo.errDesc = ERR_INVALID_NUMBER;
                curStat = NumberDfaStat::ERROR;
            }
            break;

        case NumberDfaStat::NUMBER_EXPONENT:
            if (!std::isdigit(curChar))
            {
                errInfo.errDesc = ERR_INVALID_NUMBER;
                curStat = NumberDfaStat::ERROR;
            }
            break;
        }
        returnToken.rawValue += curChar;
        _advance();
    }

    POS_T tokenLen = returnToken.rawValue.length();
    returnToken.len = tokenLen;
    errInfo.len = tokenLen;

    return curStat == NumberDfaStat::NUMBER_END;
}

bool Lexer::_parseLiteral(Token &returnToken, ErrInfo &errInfo)
{
    // 初始化返回参数
    returnToken = {"", TokenType::TRUE, _curRow, _curRow, 0};
    const POS_T lineBegin = _data.linesIndex[_curRow].first;
    const POS_T lineEnd = _data.linesIndex[_curRow].second;
    errInfo = {"", _data.source.substr(lineBegin, lineEnd - lineBegin), _curRow, _curCol, 0};

    LiteralDfaStat curStat = LiteralDfaStat::LITERAL_START;

    while (curStat != LiteralDfaStat::LITERAL_END && curStat != LiteralDfaStat::ERROR)
    {
        if (_isEndOfLine() && curStat != LiteralDfaStat::LITERAL_END)
        {
            curStat = LiteralDfaStat::ERROR;
            errInfo.errDesc = ERR_INVALID_LITERAL;
            break;
        }

        const char curChar = _current();

        switch (curStat)
        {
        case LiteralDfaStat::LITERAL_START:
            if (curChar == 't')
            {
                returnToken.type = TokenType::TRUE;
                curStat = LiteralDfaStat::TRUE_T;
            }
            else if (curChar == 'f')
            {
                returnToken.type = TokenType::FALSE;
                curStat = LiteralDfaStat::FALSE_F;
            }
            else if (curChar == 'n')
            {
                returnToken.type = TokenType::NULL_;
                curStat = LiteralDfaStat::NULL_N;
            }
            else
            {
                errInfo.errDesc = ERR_INVALID_LITERAL;
                curStat = LiteralDfaStat::ERROR;
            }
            returnToken.rawValue += curChar;
            _advance();
            break;

        case LiteralDfaStat::TRUE_T:
            if (curChar == 'r')
            {
                curStat = LiteralDfaStat::TRUE_R;
            }
            else
            {
                errInfo.errDesc = ERR_INVALID_LITERAL + std::string(LITERAL_GUESS_TRUE);
                curStat = LiteralDfaStat::ERROR;
            }
            returnToken.rawValue += curChar;
            _advance();
            break;

        case LiteralDfaStat::TRUE_R:
            if (curChar == 'u')
            {
                curStat = LiteralDfaStat::TRUE_U;
            }
            else
            {
                errInfo.errDesc = ERR_INVALID_LITERAL + std::string(LITERAL_GUESS_TRUE);
                curStat = LiteralDfaStat::ERROR;
            }
            returnToken.rawValue += curChar;
            _advance();
            break;

        case LiteralDfaStat::TRUE_U:
            if (curChar == 'e')
            {
                curStat = LiteralDfaStat::TRUE_E;
            }
            else
            {
                errInfo.errDesc = ERR_INVALID_LITERAL + std::string(LITERAL_GUESS_TRUE);
                curStat = LiteralDfaStat::ERROR;
            }
            returnToken.rawValue += curChar;
            _advance();
            break;

        case LiteralDfaStat::TRUE_E:
            if (_tokenIsOver())
            {
                curStat = LiteralDfaStat::LITERAL_END;
            }
            else
            {
                errInfo.errDesc = ERR_INVALID_LITERAL + std::string(LITERAL_GUESS_TRUE);
                curStat = LiteralDfaStat::ERROR;
            }
            break;

        case LiteralDfaStat::FALSE_F:
            if (curChar == 'a')
            {
                curStat = LiteralDfaStat::FALSE_A;
            }
            else
            {
                errInfo.errDesc = ERR_INVALID_LITERAL + std::string(LITERAL_GUESS_FALSE);
                curStat = LiteralDfaStat::ERROR;
            }
            returnToken.rawValue += curChar;
            _advance();
            break;

        case LiteralDfaStat::FALSE_A:
            if (curChar == 'l')
            {
                curStat = LiteralDfaStat::FALSE_L;
            }
            else
            {
                errInfo.errDesc = ERR_INVALID_LITERAL + std::string(LITERAL_GUESS_FALSE);
                curStat = LiteralDfaStat::ERROR;
            }
            returnToken.rawValue += curChar;
            _advance();
            break;

        case LiteralDfaStat::FALSE_L:
            if (curChar == 's')
            {
                curStat = LiteralDfaStat::FALSE_S;
            }
            else
            {
                errInfo.errDesc = ERR_INVALID_LITERAL + std::string(LITERAL_GUESS_FALSE);
                curStat = LiteralDfaStat::ERROR;
            }
            returnToken.rawValue += curChar;
            _advance();
            break;

        case LiteralDfaStat::FALSE_S:
            if (curChar == 'e')
            {
                curStat = LiteralDfaStat::FALSE_E;
            }
            else
            {
                errInfo.errDesc = ERR_INVALID_LITERAL + std::string(LITERAL_GUESS_FALSE);
                curStat = LiteralDfaStat::ERROR;
            }
            returnToken.rawValue += curChar;
            _advance();
            break;

        case LiteralDfaStat::FALSE_E:
            if (_tokenIsOver())
            {
                curStat = LiteralDfaStat::LITERAL_END;
            }
            else
            {
                errInfo.errDesc = ERR_INVALID_LITERAL + std::string(LITERAL_GUESS_FALSE);
                curStat = LiteralDfaStat::ERROR;
            }
            break;

        case LiteralDfaStat::NULL_N:
            if (curChar == 'u')
            {
                curStat = LiteralDfaStat::NULL_U;
            }
            else
            {
                errInfo.errDesc = ERR_INVALID_LITERAL + std::string(LITERAL_GUESS_NULL);
                curStat = LiteralDfaStat::ERROR;
            }
            returnToken.rawValue += curChar;
            _advance();
            break;

        case LiteralDfaStat::NULL_U:
            if (curChar == 'l')
            {
                curStat = LiteralDfaStat::NULL_L1;
            }
            else
            {
                errInfo.errDesc = ERR_INVALID_LITERAL + std::string(LITERAL_GUESS_NULL);
                curStat = LiteralDfaStat::ERROR;
            }
            returnToken.rawValue += curChar;
            _advance();
            break;

        case LiteralDfaStat::NULL_L1:
            if (curChar == 'l')
            {
                curStat = LiteralDfaStat::NULL_L2;
            }
            else
            {
                errInfo.errDesc = ERR_INVALID_LITERAL + std::string(LITERAL_GUESS_NULL);
                curStat = LiteralDfaStat::ERROR;
            }
            returnToken.rawValue += curChar;
            _advance();
            break;

        case LiteralDfaStat::NULL_L2:
            if (_tokenIsOver())
            {
                curStat = LiteralDfaStat::LITERAL_END;
            }
            else
            {
                errInfo.errDesc = ERR_INVALID_LITERAL + std::string(LITERAL_GUESS_NULL);
                curStat = LiteralDfaStat::ERROR;
            }
            break;
        }
    }

    POS_T tokenLen = returnToken.rawValue.length();
    returnToken.len = tokenLen;
    errInfo.len = tokenLen;

    return curStat == LiteralDfaStat::LITERAL_END;
}
} // namespace simpleJson
