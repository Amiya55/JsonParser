#include "lexer_parser.h"
#include "utilities.h"
#include "json_type.h"
#include <iostream>

namespace simpleJson {
    bool ErrMsgs::hasError() const noexcept {
        return !_messages.empty();
    }

    void ErrMsgs::addError(std::string &&prevLine, std::string &&curLine,
                           std::string &&nextLine, std::string &&errDesc, Token &&errToken) {
        _messages.push_back({
            std::move(curLine), std::move(prevLine), std::move(nextLine),
            std::move(errDesc), std::move(errToken)
        });
    }

    void ErrMsgs::throwError(bool throwAll) const {
        if (!hasError())
            return;

        std::string errorPrintInfo;
        for (size_t i = 0; i < 1; i++) {
            const POS_T errLineIndex = _messages[i].errToken.row + 1; // 错误所在行的行号，面对用户，索引从1开始，所以加1
            // 打印错误所在行上文
            if (!_messages[i].prevLine.empty()) {
                std::string prevLineInfo(std::to_string(errLineIndex - 1) + " | " + _messages[i].prevLine);
                errorPrintInfo.append(prevLineInfo);
            }

            // 构建错误所在行信息
            std::string LineInfo(std::to_string(errLineIndex) + " | " + _messages[i].currentLine);
            errorPrintInfo.append(LineInfo);
            // 构建空格缩进
            // 下面的errLineIndex是行号长度，数字3为字符串" | "长度，
            // _messages[i].errToken.col为本行距离错误token首字符的长度，数字1为索引差值
            std::string indent(std::to_string(errLineIndex).length() + 3 + _messages[i].errToken.col - 1, ' ');
            errorPrintInfo.append(indent);
            // 高亮错误token
            std::string highlight(_messages[i].errToken.len, '~');
            errorPrintInfo.append(highlight);
            // 添加错误描述
            errorPrintInfo.append(" " + _messages[i].errDesc + "\n");

            // 打印错误所在行下文
            if (!_messages[i].nextLine.empty()) {
                std::string nextLineInfo(std::to_string(errLineIndex + 1) + " | " + _messages[i].nextLine);
                errorPrintInfo.append(nextLineInfo);
            }

            // 构建分隔符
            errorPrintInfo.append("- - - - - - - - - - -\n");
        }

        throw std::runtime_error(errorPrintInfo);
    }

    void Lexer::_scan() {
        for (_curIndex = 0; _curIndex < _data.source.length();) {
            switch (_data.source[_curIndex]) {
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
                    const POS_T strRow = _curRow;
                    const POS_T strCol = _curCol;
                    std::string returnToken;
                    std::string errInfo;
                    if (_parseString(returnToken, errInfo)) {
                        std::cout << returnToken << std::endl;
                        // _data.tokens.push_back(_makeToken(std::move(returnToken), TokenType::STR, strRow, strCol));
                    } else {
                        std::cout << errInfo << std::endl;
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
                    const POS_T numRow = _curRow;
                    const POS_T numCol = _curCol;
                    std::string returnToken;
                    std::string errInfo;
                    if (_parseNumber(returnToken, errInfo)) {
                        std::cout << returnToken << std::endl;
                        _data.tokens.push_back(_makeToken(std::move(returnToken), TokenType::NUM, numRow, numCol));
                    } else {
                        std::cout << "error occurred! parsing number\n";
                        return;
                        // ...处理错误
                    }
                    break;
                }
                case 't':
                case 'f':
                case 'n': {
                    const POS_T liteRow = _curRow;
                    const POS_T liteCol = _curCol;
                    std::string returnToken;
                    std::string errInfo;
                    TokenType type;
                    if (_parseLiteral(returnToken, errInfo, type)) {
                        _data.tokens.push_back(_makeToken(std::move(returnToken), type, liteRow, liteCol));
                    } else {
                        std::cout << "出错了，json不支持此类令牌" << std::endl;
                        // ...处理错误
                    }
                    break;
                }
                case '\n':
                    // 额外添加分支处理换行符记录，这样不用在一开始将原始json字符串拆分，性能更好
                    ++_curRow;
                    _curCol = 0;
                    _data.linesIndex.push_back(_curIndex);
                    _advance();
                    break;
                default:
                    if (std::isspace(_data.source[_curIndex])) {
                        _advance();
                    } else {
                        // ...处理错误
                    }
            }
        }
    }

    bool Lexer::_dfaDone(char curChar) const noexcept {
        return !_isAtEnd() && (std::isspace(curChar) ||
               curChar == ']' || curChar == '\0' ||
               curChar == '}' || curChar == ',');
    }


    bool Lexer::_isAtEnd() const noexcept {
        return _curIndex == _data.source.length();
    }

    bool Lexer::_isEndOfLine() const noexcept {
        return _data.source[_curIndex] == '\n';
    }

    char Lexer::_prev() const noexcept {
        if (_curIndex > 0) {
            return _data.source[_curIndex - 1];
        }
        return '\0';
    }

    char Lexer::_current() const noexcept {
        return _data.source[_curIndex];
    }

    char Lexer::_peek() const noexcept {
        if (!_isAtEnd()) {
            return _data.source[_curIndex + 1];
        }
        return '\0';
    }

    char Lexer::_advance() noexcept {
        if (!_isAtEnd()) {
            ++_curCol;
            return _data.source[++_curIndex];
        }
        return '\0';
    }

    Token Lexer::_makeToken(std::string &&str, TokenType type, POS_T row, POS_T col) noexcept {
        const LENGTH_T tokenLen = str.length();
        return {std::move(str), type, row, col, tokenLen};
    }

    bool Lexer::_parseString(std::string &returnToken, std::string& errInfo) {
        returnToken.clear();
        errInfo.clear();

        StringDfaStat curStat = StringDfaStat::STRING_START;
        std::string unicode_buffer;

        while (curStat != StringDfaStat::STRING_END &&
               curStat != StringDfaStat::ERROR) {
            if (_isEndOfLine()) {
                // 在非StringDfaStat::STRING_END情况下结束一行，意味着json字符串没有被引号括起来
                curStat = StringDfaStat::ERROR;
                errInfo = ERR_MISSING_QUOTATION_MARK;
                break;
            }

            const char curChar = _current();

            switch (curStat) {
                case StringDfaStat::STRING_START:
                    if (curChar == '"') {
                        curStat = StringDfaStat::IN_STRING;
                    } else {
                        curStat = StringDfaStat::ERROR;
                        errInfo = ERR_MISSING_QUOTATION_MARK;
                    }
                    _advance();
                    break;

                case StringDfaStat::IN_STRING:
                    if (curChar == '"') {
                        curStat = StringDfaStat::STRING_END;
                    } else if (curChar == '\\') {
                        curStat = StringDfaStat::STRING_ESCAPE;
                    } else {
                        returnToken += curChar;
                    }
                    _advance();
                    break;

                case StringDfaStat::STRING_ESCAPE:
                    switch (curChar) {
                        case '\\': returnToken += '\\'; curStat = StringDfaStat::IN_STRING; break;
                        case '"': returnToken += '"'; curStat = StringDfaStat::IN_STRING; break;
                        case '/': returnToken += '/'; curStat = StringDfaStat::IN_STRING; break;
                        case 'b': returnToken += '\b'; curStat = StringDfaStat::IN_STRING; break;
                        case 'f': returnToken += '\f'; curStat = StringDfaStat::IN_STRING; break;
                        case 'r': returnToken += '\r'; curStat = StringDfaStat::IN_STRING; break;
                        case 'n': returnToken += '\n'; curStat = StringDfaStat::IN_STRING; break;
                        case 't': returnToken += '\t'; curStat = StringDfaStat::IN_STRING; break;
                        case 'u':
                            curStat = StringDfaStat::STRING_UNICODE_START;
                            unicode_buffer = "\\u";
                            break;
                        default:
                            curStat = StringDfaStat::ERROR;
                            errInfo = ERR_INVALID_ESCAPE + '\\' + curChar;
                            break;
                    }
                    _advance();
                    break;

                case StringDfaStat::STRING_UNICODE_START:
                    for (int i = 0; i < 4; ++i, _advance()) {
                        if (_isEndOfLine()) {
                            curStat = StringDfaStat::ERROR;
                            errInfo = ERR_MISSING_QUOTATION_MARK;
                            break;
                        }

                        if (std::isdigit(_current()) ||
                            (_current() >= 'a' && _current() <= 'f') ||
                            (_current() >= 'A' && _current() <= 'F')) {
                            unicode_buffer += _current();
                        } else {
                            curStat = StringDfaStat::ERROR;
                            errInfo = ERR_INVALID_ESCAPE;
                            break;;
                        }
                    }
                    returnToken += convert_unicode_escape(unicode_buffer);
                    curStat = StringDfaStat::IN_STRING;
                    break;
            }
        }

        return curStat == StringDfaStat::STRING_END;
    }

    bool Lexer::_parseNumber(std::string &returnToken, std::string& errInfo) {
        DfaStat curStat;
        if (const char ch = _current(); ch == '-') {
            curStat = DfaStat::NumberSign;
        } else if (ch == '0') {
            curStat = DfaStat::NumberZero;
        } else if (std::isdigit(ch)) {
            curStat = DfaStat::NumberIntegral;
        } else {
            return false;
        }

        while (true) {
            switch (curStat) {
                case DfaStat::NumberSign:
                    if (std::isdigit(_peek()))
                        curStat = DfaStat::NumberIntegral;
                    else
                        return false;

                    returnToken.push_back(_current());
                    _advance();
                    break;
                case DfaStat::NumberZero:
                    if (const char nextChar = _peek(); nextChar == '.')
                        curStat = DfaStat::NumberFractionBegin;
                    else if (nextChar == 'e')
                        curStat = DfaStat::NumberExponentBegin;
                    else if (_dfaDone(nextChar))
                        curStat = DfaStat::NumberEnd;
                    else
                        return false;

                    returnToken.push_back(_current());
                    _advance();
                    break;
                case DfaStat::NumberIntegral:
                    if (const char nextChar = _peek(); nextChar == '.') {
                        curStat = DfaStat::NumberFractionBegin;
                    }
                    else if (nextChar == 'e') {
                        curStat = DfaStat::NumberExponentBegin;
                    }
                    else if (_dfaDone(nextChar)) {
                        curStat = DfaStat::NumberEnd;
                    }
                    else {
                        if (!std::isdigit(nextChar))
                            return false;
                    }

                    returnToken.push_back(_current());
                    _advance();
                    break;
                case DfaStat::NumberFractionBegin:
                    if (const char nextChar = _peek(); std::isdigit(nextChar))
                        curStat = DfaStat::NumberFraction;
                    else
                        return false;

                    returnToken.push_back(_current());
                    _advance();
                    break;
                case DfaStat::NumberFraction:
                    if (const char nextChar = _peek(); nextChar == 'e') {
                        curStat = DfaStat::NumberExponentBegin;
                    }
                    else if (_dfaDone(nextChar)) {
                        curStat = DfaStat::NumberEnd;
                    }
                    else {
                        if (!std::isdigit(nextChar))
                            return false;
                    }

                    returnToken.push_back(_current());
                    _advance();
                    break;
                case DfaStat::NumberExponentBegin:
                    if (const char nextChar = _peek(); nextChar == '-')
                        curStat = DfaStat::NumberExponentSign;
                    else if (std::isdigit(nextChar))
                        curStat = DfaStat::NumberExponent;
                    else
                        return false;

                    returnToken.push_back(_current());
                    _advance();
                    break;
                case DfaStat::NumberExponentSign:
                    if (const char nextChar = _peek(); std::isdigit(nextChar))
                        curStat = DfaStat::NumberExponent;
                    else
                        return false;

                    returnToken.push_back(_current());
                    _advance();
                    break;
                case DfaStat::NumberExponent:
                    if (const char nextChar = _peek(); std::isdigit(nextChar))
                        curStat = DfaStat::NumberExponent;
                    else if (_dfaDone(nextChar))
                        curStat = DfaStat::NumberEnd;
                    else
                        return false;

                    returnToken.push_back(_current());
                    _advance();
                    break;
                case DfaStat::NumberEnd:
                    return true;
            }
        }
    }

    bool Lexer::_parseLiteral(std::string &returnToken, std::string& errInfo, TokenType &type) {
        DfaStat curStat;
        if (const char ch = _current(); ch == 't') {
            type = TokenType::TRUE;
            curStat = DfaStat::TrueT;
            returnToken.push_back('t');
        } else if (ch == 'f') {
            type = TokenType::FALSE;
            curStat = DfaStat::FalseF;
            returnToken.push_back('f');
        } else if (ch == 'n') {
            type = TokenType::NULL_;
            curStat = DfaStat::NullN;
            returnToken.push_back('n');
        } else {
            return false;
        }

        while (true) {
            switch (curStat) {
                case DfaStat::TrueT:
                    if (_peek() == 'r') _advance();
                    else return false;
                    curStat = DfaStat::TrueR;
                    returnToken.push_back('r');
                    break;
                case DfaStat::TrueR:
                    if (_peek() == 'u') _advance();
                    else return false;
                    curStat = DfaStat::TrueU;
                    returnToken.push_back('u');
                    break;
                case DfaStat::TrueU:
                    if (_peek() == 'e') _advance();
                    else return false;
                    curStat = DfaStat::TrueE;
                    returnToken.push_back('e');
                    break;
                case DfaStat::TrueE: {
                    if (const char nextChar = _peek(); _dfaDone(nextChar)) {
                        _advance();
                        return true;
                    }
                    return false;
                }

                case DfaStat::FalseF:
                    if (_peek() == 'a') _advance();
                    else return false;
                    curStat = DfaStat::FalseA;
                    returnToken.push_back('a');
                    break;
                case DfaStat::FalseA:
                    if (_peek() == 'l') _advance();
                    else return false;
                    curStat = DfaStat::FalseL;
                    returnToken.push_back('l');
                    break;
                case DfaStat::FalseL:
                    if (_peek() == 's') _advance();
                    else return false;
                    curStat = DfaStat::FalseS;
                    returnToken.push_back('s');
                    break;
                case DfaStat::FalseS:
                    if (_peek() == 'e') _advance();
                    else return false;
                    curStat = DfaStat::FalseE;
                    returnToken.push_back('e');
                    break;
                case DfaStat::FalseE: {
                    if (const char nextChar = _peek(); _dfaDone(nextChar)) {
                        _advance();
                        return true;
                    }
                    return false;
                }

                case DfaStat::NullN:
                    if (_peek() == 'u') _advance();
                    else return false;
                    curStat = DfaStat::NullU;
                    returnToken.push_back('u');
                    break;
                case DfaStat::NullU:
                    if (_peek() == 'l') _advance();
                    else return false;
                    curStat = DfaStat::NullL1;
                    returnToken.push_back('l');
                    break;
                case DfaStat::NullL1:
                    if (_peek() == 'l') _advance();
                    else return false;
                    curStat = DfaStat::NullL2;
                    returnToken.push_back('l');
                    break;
                case DfaStat::NullL2:
                    if (const char nextChar = _peek(); _dfaDone(nextChar)) {
                        _advance();
                        return true;
                    }
                    return false;
            }
        }
    }
}
