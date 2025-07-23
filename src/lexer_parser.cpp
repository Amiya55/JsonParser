#include "lexer_parser.h"

#include "json_type.h"

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
                        _data.tokens.push_back(_makeToken(std::move(returnToken), TokenType::STR));
                    } else {
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
                        _data.tokens.push_back(_makeToken(std::move(returnToken), TokenType::NUM));
                    } else {
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

    bool Lexer::_isAtEnd() const noexcept {
        return _curIndex == _data.source.length();
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
        return {std::move(str), type, _curRow, _curCol, str.length()};
    }


    bool Lexer::_parseString(std::string &returnToken) {
    }

    bool Lexer::_parseNumber(std::string &returnToken) {
    }

    bool Lexer::_parseLiteral(std::string &returnToken, TokenType &type) {
    }
}
