#include "lexer_parser.h"

#include "json_type.h"

namespace simpleJson {
    POS_T _curIndex{0}; // 当前在原始json字符串中的索引
    POS_T _curRow{0}; // 当前字符行
    POS_T _curCol{0}; // 当前字符列

    void Lexer::_scan() {
        for (_curIndex = 0; _curIndex < _data.source.length();) {
            switch (_data.source[_curIndex]) {
                case '{':
                    _data.tokens.push_back({
                        "{", TokenType::LBRACE, _curRow, _curCol, 1
                    });
                    break;
                case '}':
                    _data.tokens.push_back({
                        "}", TokenType::RBRACE, _curRow, _curCol, 1
                    });
                    break;
                case '[':
                    _data.tokens.push_back({
                        "[", TokenType::LBRACKET, _curRow, _curCol, 1
                    });
                    break;
                case ']':
                    _data.tokens.push_back({
                        "]", TokenType::RBRACKET, _curRow, _curCol, 1
                    });
                    break;
                case ',':
                    _data.tokens.push_back({
                        ",", TokenType::COMMA, _curRow, _curCol, 1
                    });
                    break;
                case ':':
                    _data.tokens.push_back({
                        ":", TokenType::COLON, _curRow, _curCol, 1
                    });
                    break;
                case '\"': {
                    std::string returnToken;
                    if (POS_T skipLen = _parseString(returnToken) != POS_OVERGLOW) {
                        const POS_T tokenLen = returnToken.length();
                        _data.tokens.push_back({
                            std::move(returnToken), TokenType::STR, _curRow, _curCol, tokenLen
                        });
                        _curIndex += tokenLen;
                        _curCol += tokenLen;
                    } else {
                        // ...处理错误
                    }
                }
                    break;
                case '-':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9': {
                    std::string returnToken;
                    if (POS_T ret = _parseNumber(returnToken) == POS_OVERGLOW) {
                        const POS_T tokenLen = returnToken.length();
                        _data.tokens.push_back({
                            std::move(returnToken), TokenType::NUM, _curRow, _curCol, tokenLen
                        });
                        _curIndex += tokenLen;
                        _curCol += tokenLen;
                    } else {
                        // ...处理错误
                    }
                }
                    break;
                case 't':
                case 'f':
                case 'n': {
                    std::string returnToken;
                    if (POS_T ret = _parseLiteral(returnToken) == POS_OVERGLOW) {
                        const POS_T tokenLen = returnToken.length();
                        _data.tokens.push_back({
                            std::move(returnToken), ? , _curRow, _curCol, tokenLen
                        });
                    } else {
                        // ...处理错误
                    }
                }
                    break;
                case '\n':
                    ++_curIndex;
                    _data.linesIndex.push_back(_curIndex);
                    ++_curRow;
                    _curCol = 0;
                    break;
                default:

            }
        }
    }

    POS_T Lexer::_parseString(std::string &returnToken) {
    }

    POS_T Lexer::_parseNumber(std::string &returnToken) {
    }

    POS_T Lexer::_parseLiteral(std::string &returnToken) {
    }
}
