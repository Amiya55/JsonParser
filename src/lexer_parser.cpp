#include "lexer_parser.h"
#include "utilities.h"
#include "json_type.h"
#include <iostream>

namespace simpleJson {
    void Lexer::_scan() {
        for (_curIndex = 0; _curIndex < _data.source.length();) {
            switch (_data.source[_curIndex]) {
                case '{':
                    _data.tokens.push_back(_makeToken("{", TokenType::LBRACE));
                    _advance();
                    break;
                case '}':
                    _data.tokens.push_back(_makeToken("{", TokenType::RBRACE));
                    _advance();
                    break;
                case '[':
                    _data.tokens.push_back(_makeToken("[", TokenType::LBRACKET));
                    _advance();
                    break;
                case ']':
                    _data.tokens.push_back(_makeToken("[", TokenType::RBRACKET));
                    _advance();
                    break;
                case ',':
                    _data.tokens.push_back(_makeToken(",", TokenType::COMMA));
                    _advance();
                    break;
                case ':':
                    _data.tokens.push_back(_makeToken(":", TokenType::COLON));
                    _advance();
                    break;
                case '\"': {
                    if (std::string returnToken; _parseString(returnToken)) {
                        std::cout << returnToken << std::endl;
                        _data.tokens.push_back(_makeToken(std::move(returnToken), TokenType::STR));
                    } else {
                        std::cout << "error occurred!\n";
                        // ...处理错误
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
                    if (std::string returnToken; _parseNumber(returnToken)) {
                        std::cout << returnToken << std::endl;
                        _data.tokens.push_back(_makeToken(std::move(returnToken), TokenType::NUM));
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
                    std::string returnToken;
                    TokenType type;
                    if (_parseLiteral(returnToken, type)) {
                        _data.tokens.push_back(_makeToken(std::move(returnToken), type));
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

    Token Lexer::_makeToken(std::string &&str, TokenType type) const noexcept {
        const LENGTH_T tokenLen = str.length();
        return {std::move(str), type, _curRow, _curCol, tokenLen};
    }


    bool Lexer::_parseString(std::string &returnToken) {
        DfaStat curStat;
        bool hasUnicodeEscape = false; // 如果存在unicode转义序列，需要进行向utf8的转换
        if (const char ch = _current(); ch == '\"') {
            curStat = DfaStat::InString;
            _advance();
        } else {
            return false;
        }

        while (true) {
            switch (curStat) {
                case DfaStat::InString:
                    if (_current() == '\"' && _prev() != '\\') {
                        curStat = DfaStat::EndString;
                        break;
                    }

                    if (_current() == '\\') {
                        switch (_peek()) {
                            case '/':
                                _advance();
                                break;
                            case 'u':
                                hasUnicodeEscape = true;
                                curStat = DfaStat::StringEscape;
                            case '\\':
                            case '\"':
                            case 'n':
                            case 't':
                            case 'b':
                            case 'f':
                            case 'r':
                                returnToken.push_back('\\');
                                _advance();
                                break;
                            default:
                                return false;
                        }
                    }
                    returnToken.push_back(_current());
                    _advance();
                    break;
                case DfaStat::StringEscape: {
                    constexpr int unicodeEscapeLen = 4; // unicode转义序列字符数为4，例如\u4e00
                    for (int i = 0; i < unicodeEscapeLen; ++i) {
                        if (std::isdigit(_current()) ||
                            (_current() >= 'a' && _current() <= 'f') ||
                            (_current() >= 'A' && _current() <= 'F')) {
                            returnToken.push_back(_current());
                            _advance();
                        } else {
                            return false;
                        }
                    }
                    curStat = DfaStat::InString;
                    break;
                }
                case DfaStat::EndString:
                    if (const char nextChar = _peek(); _dfaDone(nextChar) || nextChar == ':') {
                        _advance();
                        if (hasUnicodeEscape)
                            returnToken = convert_unicode_escape(returnToken);
                        return true;
                    }
                    return false;
            }
        }
    }

    bool Lexer::_parseNumber(std::string &returnToken) {
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

    bool Lexer::_parseLiteral(std::string &returnToken, TokenType &type) {
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
