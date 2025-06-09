#include "jsonParser.h"
#include <algorithm>
#include <cctype>
#include <cstddef>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stack>
#include <stdexcept>
#include <string>
#include "jsonTypes.h"
#include "utilities.h"

namespace simpleJson {
    /* Token */
    Token::Token(TokenType type, const std::string &value,
                 size_t line, size_t column)
        : _type(type)
          , _value(value)
          , _line(line)
          , _column(column) {
    }

    TokenType Token::type() const {
        return _type;
    }


    /* Lexer */
    Lexer::Lexer(const std::string &input) {
        if (input.empty()) {
            throw std::invalid_argument("Lexer: empty input");
        }

        size_t begin = 0;
        size_t end = 0;
        while (begin < input.size()) {
            end = input.find_first_of('\n', begin);
            if (end == std::string::npos) {
                _input.emplace_back(input.substr(begin));
                break;
            }

            _input.emplace_back(input.substr(begin, end - begin));
            begin = end + 1;
        }
    }

    void Lexer::peekToken() {
        for (; _line < _input.size(); ++_line, _column = 0) {
            for (; _column < _input[_line].size(); ++_column) {
                _skipWhitespace();
                if (_line >= _input.size()) // 判断读取到的是否为末尾的空白字符
                    break;

                switch (_input[_line][_column]) {
                    case '{':
                        _tokens.emplace_back(
                            TokenType::LBRACE, "{",
                            _line, _column);
                        break;
                    case '}':
                        _tokens.emplace_back(
                            TokenType::RBRACE, "}",
                            _line, _column);
                        break;
                    case '[':
                        _tokens.emplace_back(
                            TokenType::LBRACKET, "[",
                            _line, _column);
                        break;
                    case ']':
                        _tokens.emplace_back(
                            TokenType::RBRACKET, "]",
                            _line, _column);
                        break;
                    case ':':
                        _tokens.emplace_back(
                            TokenType::COLON, ":",
                            _line, _column);
                        break;
                    case ',':
                        _tokens.emplace_back(
                            TokenType::COMMA, ",",
                            _line, _column);
                        break;
                    case '\"':
                        _tokens.emplace_back(_parseString());
                        break;
                    case '-':
                        _tokens.emplace_back(_parseNumber());
                        break;
                    default: {
                        if (std::isspace(_input[_line][_column]))
                            break;

                        if (std::isalpha(_input[_line][_column])) {
                            _tokens.emplace_back(_parseKeyword());
                        } else if (std::isdigit(_input[_line][_column])) {
                            _tokens.emplace_back(_parseNumber());
                        } else {
                            const size_t start = _column;
                            while (_column < _input[_line].size() && _input[_line][_column] != ',' &&
                                   _input[_line][_column] != '}' && _input[_line][_column] != ']' &&
                                   !isspace(_input[_line][_column])) {
                                ++_column;
                            }

                            throw std::invalid_argument(_buildErrMsg(
                                "invalid json value", _line,
                                start, _column - 1));
                        }
                    }
                }
            }
        }
    }

    const std::vector<Token> &Lexer::getTokens() const noexcept {
        return _tokens;
    }

    const std::vector<std::string> &Lexer::getInput() const noexcept {
        return _input;
    }

    void Lexer::_skipWhitespace() noexcept {
        while (_line < _input.size() &&
               std::isspace(_input[_line][_column])) {
            ++_column;
            if (_column >= _input[_line].size()) {
                ++_line;
                _column = 0;
            }
        }
    }

    Token Lexer::_parseString() {
        size_t start = _column; // 当前字符串的起始位置
        ++_column; // 对_column加加，跳过左边的\"引号
        std::string token("\"");
        size_t curQueStart = 0; // 当前转义序列起始位置(配合实现更精准的错误高亮打印)

        while (_column < _input[_line].size()) {
            if (_input[_line][_column] == '\"' && _input[_line][_column - 1] != '\\') {
                /* 一旦遇到右边的\"引号，直接跳出循环 */
                token += "\"";
                break;
            }

            if (_input[_line][_column] == '\\' && _column + 1 < _input[_line].size()) {
                /* 分析转义字符是否合法 */
                if (_input[_line][_column + 1] == 'u') {
                    token += "\\u";
                    curQueStart = _column; // 当前的转义序列起始位置(配合错误高亮打印)
                    _column += 2;
                    bool isValid = true;

                    size_t count = 0;
                    for (count = 0; count < 4 && _column < _input[_line].size(); ++count, ++_column) {
                        if (!std::isdigit(_input[_line][_column]) &&
                            !(_input[_line][_column] >= 'a' && _input[_line][_column] <= 'f') &&
                            !(_input[_line][_column] >= 'A' && _input[_line][_column] <= 'F')) {
                            /* Unicode序列必须是四位16进制字符序列 */
                            isValid = false;
                        }
                        token += _input[_line][_column];
                    }

                    if (count < 4 || isValid == false) {
                        throw std::invalid_argument(_buildErrMsg(
                            "invalid unicode escape sequence", _line,
                            curQueStart, _column - 1));
                    }

                    if (count == 4) {
                        continue;
                    }
                } else if (_input[_line][_column + 1] == '\\') {
                    /* 针对两个反斜杠连续出现的问题:
                     * \m非法，但是\\m是合法的，因为第二个反斜杠与第一个配对而不是与m配对 */
                    token += "\\"; // 直接将当前的反斜杠添加到token中
                    ++_column; // 对_column加加一次，跳过第二个反斜杠，再由循环末尾的++_column将第二个反斜杠添加到token中
                } else {
                    if (_input[_line][_column + 1] != 'n' && _input[_line][_column + 1] != 't' &&
                        _input[_line][_column + 1] != 'f' && _input[_line][_column + 1] != 'b' &&
                        _input[_line][_column + 1] != 'r' && _input[_line][_column + 1] != '/' &&
                        _input[_line][_column + 1] != '\"') {
                        curQueStart = _column;
                        _column += 2;
                        throw std::invalid_argument(_buildErrMsg(
                            "Invalid escape sequence", _line,
                            curQueStart, _column - 1));
                    }
                }
            }

            token += _input[_line][_column];
            ++_column;
        }

        if (token.back() != '\"') {
            /* 出循环，两种可能: 遇到了右边的\"引号和到达一行的末尾。
             * 如果到达一行的末尾还没有遇到右边的引号，说明字符串没有关闭 */
            throw std::invalid_argument(_buildErrMsg(
                "lack of right quotation marks", _line,
                start, _column - 1));
        }

        return {TokenType::STRING, token, _line, start};
    }

    Token Lexer::_parseNumber() {
        std::string token;
        size_t start = _column;

        bool isValid = true;
        bool eExist = false; // 标记e的数量
        bool pointExist = false; // 标记小数点的数量
        while (_column < _input[_line].size() && !std::isspace(_input[_line][_column]) &&
               _input[_line][_column] != ',' && _input[_line][_column] != '}' &&
               _input[_line][_column] != ']') {
            if (_input[_line][_column] == '-') {
                if ((_column != start && _input[_line][_column - 1] != 'e') ||
                    !isdigit(_input[_line][_column + 1])) {
                    isValid = false;
                }
            } else if (_input[_line][_column] == '.') {
                if (pointExist == false && // 必须只有一个小数点
                    eExist == false && // 针对小数点的特殊情况，小数点不能在e之后出现
                    _column != start && isdigit(_input[_line][_column - 1]) && // 小数点前必须是数字
                    isdigit(_input[_line][_column + 1])) {
                    // 小数点后必须是数字，这里不检查越界访问，因为string最后一位是'\0'
                    pointExist = true;
                } else {
                    isValid = false;
                }
            } else if (_input[_line][_column] == 'e') {
                if (eExist == false && // 必须只有一个e
                    _column != start && isdigit(_input[_line][_column - 1]) && // e前必须是数字
                    (isdigit(_input[_line][_column + 1]) || _input[_line][_column + 1] == '-')) {
                    // e后可以是数字，也可以是负号，因为存在123e-3的情况
                    eExist = true;
                } else {
                    isValid = false;
                }
            } else {
                if (!isdigit(_input[_line][_column])) {
                    isValid = false;
                }
            }

            token += _input[_line][_column];
            ++_column;
        }

        if (isValid == false) {
            throw std::invalid_argument(_buildErrMsg(
                "invalid json number", _line,
                start, _column - 1));
        }

        if (_input[_line][_column] == ',' || _input[_line][_column] == '}' ||
            _input[_line][_column] == ']') {
            /* 针对遇到逗号和右括号的情况，必须减减_column，否则会漏掉当前的字符 */
            --_column;
        }

        return {TokenType::NUMBER, token, _line, start};
    }

    Token Lexer::_parseKeyword() {
        std::string token;
        size_t start = _column;
        while (!std::isspace(_input[_line][_column]) &&
               _column < _input[_line].size() && _input[_line][_column] != ',' &&
               _input[_line][_column] != '}' && _input[_line][_column] != ']') {
            token += _input[_line][_column];
            ++_column;
        }

        TokenType type;
        if (token == "false") {
            type = TokenType::FALSE;
        } else if (token == "true") {
            type = TokenType::TRUE;
        } else if (token == "null") {
            type = TokenType::NULL_;
        } else {
            throw std::invalid_argument(_buildErrMsg(
                "invalid json value", _line,
                start, _column - 1));
        }

        if (_input[_line][_column] == ',' || _input[_line][_column] == '}' ||
            _input[_line][_column] == ']') {
            /* 针对遇到逗号和右括号的情况，必须减减_column，否则会漏掉当前的字符 */
            --_column;
        }

        return {type, token, _line, start};
    }

    std::string Lexer::_buildErrMsg(
        std::string &&msg, size_t line,
        size_t highlightBegin, size_t highlightEnd) const noexcept {
        const std::string rowNum = std::to_string(line + 1);
        std::string errMsg(rowNum + " | ");
        errMsg += _input[line] + "\n" + std::string(rowNum.size() + 3, ' ');
        for (size_t i = 0; i < _input[line].size(); ++i) {
            if (i > highlightEnd)
                break;
            errMsg += i >= highlightBegin ? '^' : ' ';
        }
        errMsg += "  " + msg;

        return errMsg;
    }

    /* Parser */
    Parser::Parser(const Lexer &lexer) noexcept
        : _lexer(lexer)
          , _curIndex(0)
          , _curToken(_lexer.getTokens()[_curIndex]) {
    }

    void Parser::parse() {
        if (_curToken.type() == TokenType::LBRACE) {
            _advance();
            _ast = _parseObject();
        } else if (_curToken.type() == TokenType::LBRACKET) {
            _advance();
            _ast = _parseArray();
        } else {
            if (_curIndex == 0) {
                throw std::invalid_argument(_buildErrMsg(
                    "the top layer of json must be an object or an array", _curToken._column));
            }

            _advance();
        }
    }

    JsonValue Parser::_parseValue() {
        switch (_curToken.type()) {
            case TokenType::LBRACE:
                _advance();
                return _parseObject();
            case TokenType::LBRACKET:
                _advance();
                return _parseArray();
            case TokenType::NUMBER:
                if (_curToken._value.find('.') != std::string::npos ||
                    _curToken._value.find("e-") != std::string::npos) {
                    return JsonValue(std::stod(_curToken._value));
                }
                return JsonValue(std::stoi(_curToken._value));
            case TokenType::STRING:
                return JsonValue(_curToken._value);
            case TokenType::FALSE:
                return JsonValue(false);
            case TokenType::TRUE:
                return JsonValue(true);
            case TokenType::NULL_:
                return JsonValue();
            default:
                throw std::invalid_argument(_buildErrMsg("invalid value", _curToken._column));
        }
    }


    JsonValue Parser::_parseObject() {
        std::unordered_map<std::string, JsonValue> obj;

        while (_curIndex < _lexer.getTokens().size() && _curToken.type() != TokenType::RBRACE) {
            std::string key(_curToken._value);
            if (_consume(TokenType::STRING)) {
                _advance();
            } else {
                throw std::invalid_argument(_buildErrMsg(
                    "the json object expected a string as key", _curToken._column));
            }

            if (_consume(TokenType::COLON)) {
                _advance();
            } else {
                throw std::invalid_argument(_buildErrMsg(
                    "the json object expected key-value module, here should be :", _curToken._column));
            }

            const JsonValue val = _parseValue();
            obj[key] = val;
            _advance();

            if (_match(TokenType::COMMA)) {
                _advance();
                if (_peek().type() == TokenType::RBRACE) {
                    throw std::invalid_argument(_buildErrMsg(
                        "json standard does not allow the use of trailing commas", _curToken._column - 1));
                }
            } else {
                if (_peek().type() != TokenType::RBRACE) {
                    throw std::invalid_argument(_buildErrMsg(
                        "expected ',' or '}'", _curToken._column));
                }
            }
        }

        return JsonValue(obj);
    }

    JsonValue Parser::_parseArray() {
        std::vector<JsonValue> arr;

        while (_curToken.type() != TokenType::RBRACKET) {
            const JsonValue val = _parseValue();
            arr.push_back(val);
            _advance();

            if (_match(TokenType::COMMA)) {
                _advance();
                if (_peek().type() == TokenType::RBRACKET) {
                    throw std::invalid_argument(_buildErrMsg(
                        "json standard does not allow the use of trailing commas", _curToken._column));
                }
            } else {
                _advance();
                if (_peek().type() != TokenType::RBRACKET) {
                    throw std::invalid_argument(_buildErrMsg(
                        "expected ',' or ']'", _curToken._column));
                }
            }
        }

        return JsonValue(arr);
    }

    bool Parser::_consume(TokenType expected) const noexcept {
        return _curToken.type() == expected;
    }

    const Token &Parser::_peek() const {
        if (_curIndex < _lexer.getTokens().size()) {
            return _lexer.getTokens()[_curIndex];
        }
        throw std::out_of_range("Parser::_peek() -> _curIndex out of range");
    }

    const Token &Parser::_advance() {
        if (_curIndex < _lexer.getTokens().size()) {
            ++_curIndex; // 向前走一步
            _curToken = _lexer.getTokens()[_curIndex];
            return _lexer.getTokens()[_curIndex - 1]; // 返回为向前探测的值类似于后置++
        }
        throw std::invalid_argument("Parser::_advance() -> _curIndex out of range");
    }

    bool Parser::_match(TokenType type) const noexcept {
        return _curToken.type() == type;
    }


    std::string Parser::_buildErrMsg(std::string &&msg, size_t highlightBegin) const noexcept {
        const size_t line = _curToken._line;

        const std::string rowNum = std::to_string(line + 1);
        std::string errMsg(rowNum + " | ");
        errMsg += _lexer.getInput()[line] + "\n" + std::string(rowNum.size() + 3, ' ');
        for (size_t i = 0; i < _lexer.getInput()[line].size(); ++i) {
            if (i > highlightBegin)
                break;
            errMsg += i >= highlightBegin ? '^' : ' ';
        }
        errMsg += "  " + msg;

        return errMsg;
    }
}
