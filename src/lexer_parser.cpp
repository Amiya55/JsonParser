#include "lexer_parser.h"

namespace simpleJson {
    POS_T _curIndex{0}; // 当前在原始json字符串中的索引
    POS_T _curRow{0}; // 当前字符行
    POS_T _curCol{0}; // 当前字符列

    void Lexer::_scan() {
        for (_curIndex = 0; _curIndex < _data.source.length(); ) {
            switch (_data.source[_curIndex]) {
                case '{':
                    break;
                case '[':
                    break;
                case ',':
                    break;
                case ':':
                    break;
                case '\"':
                    break;
                case '-':
                    break;
                case '\n':
                    ++_curIndex;
                    _data.linesIndex.push_back(_curIndex);
                    ++_curRow;
                    _curCol = 0;
                    break;
                default:
                    if (std::isdigit(_data.source[_curIndex])) {

                    } else if (std::isspace(_data.source[_curIndex])) {

                    } else {
                        // 错误
                    }
            }
        }
    }

    POS_T Lexer::_parseObject() {

    }

    POS_T Lexer::_parseArray() {

    }

    POS_T Lexer::_parseString() {

    }

    POS_T Lexer::_parseNumber() {

    }

    POS_T Lexer::_parseLiteral() {

    }

}