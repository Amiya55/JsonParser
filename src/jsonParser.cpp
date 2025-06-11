#include "jsonParser.h"
#include "jsonTypes.h"
#include <cctype>
#include <fstream>
#include <stdexcept>
#include <string>


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

        _splitToken();
    }

    void Lexer::_splitToken() {
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
        const size_t indent = errMsg.size();

        errMsg += _input[line] + "\n" + std::string(rowNum.size() + 3, ' ');
        for (size_t i = 0; i < _input[line].size() + indent; ++i) {
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
          , _curToken(_lexer.getTokens()[_curIndex])
          , _ast(new JsonValue) {
    }

    void Parser::parse() {
        // json文件最顶层必须是[]或者{}
        switch (_curToken.type()) {
            case TokenType::LBRACE:
                *_ast = _parseObject();
                break;
            case TokenType::LBRACKET:
                *_ast = _parseArray();
                break;
            default: {
                const std::string curValMsg("'" + _peek()._value + "'");
                throw std::invalid_argument(_buildErrMsg(
                    "the top layer of json must be an object or an array, "
                    "here should be '[' or '{', but got a " + curValMsg, _peek()));
            }
        }
    }

    std::shared_ptr<JsonValue> Parser::getAst() noexcept {
        return _ast;
    }

    JsonValue Parser::_parseValue() {
        switch (_curToken.type()) {
            case TokenType::LBRACE:
                return _parseObject();
            case TokenType::LBRACKET:
                return _parseArray();
            case TokenType::NUMBER:
                if (_curToken._value.find('.') != std::string::npos ||
                    _curToken._value.find('e') != std::string::npos) {
                    return JsonValue(std::stod(_curToken._value));
                }
                return JsonValue(std::stoll(_curToken._value));
            case TokenType::STRING:
                return JsonValue(_curToken._value);
            case TokenType::FALSE:
                return JsonValue(false);
            case TokenType::TRUE:
                return JsonValue(true);
            case TokenType::NULL_:
                return JsonValue();
            default: {
                // 首先构建已经得到的token的信息
                const std::string curValMsg("'" + _peek()._value + "'" +
                                            " (at line " + std::to_string(_peek()._line + 1) + ")");
                throw std::invalid_argument(_buildErrMsg(
                    "expected a valid value here, but got a " + curValMsg,
                    _peekPrev(), _peekPrev()._value.size()));
            }
        }
    }


    JsonValue Parser::_parseObject() {
        // 首先跳过左括号'{'
        _advance();
        if (_isAtEnd()) {
            throw std::invalid_argument(_buildErrMsg(
                "expected json object <key> or '}', but now we meet EOF",
                _peekPrev()));
        }

        std::unordered_map<std::string, JsonValue> obj;

        while (!_isAtEnd() && !_match(_peek(), TokenType::RBRACE)) {
            std::string key(_curToken._value);

            if (!_isAtEnd() && _match(_peek(), TokenType::STRING)) {
                _advance();
            } else {
                throw std::invalid_argument(_buildErrMsg(
                    "the json object expected a string as key",
                    _peek()));
            }

            if (!_isAtEnd() && _match(_peek(), TokenType::COLON)) {
                _advance();
            } else {
                throw std::invalid_argument(_buildErrMsg(
                    "the json object expected key-value module, here should be :",
                    _peekPrev(), _peekPrev()._value.size()));
            }

            const JsonValue val = _parseValue();
            obj[key] = val;
            _advance();

            if (!_isAtEnd() && _match(_peek(), TokenType::COMMA)) {
                _advance();
                if (_isAtEnd()) {
                    // 对应情况: {"hello": null, 缺少右括号
                    throw std::invalid_argument(_buildErrMsg(
                        "expected <key> or replace the comma to '}'", _peekPrev(),
                        _peekPrev()._value.size()));
                }

                if (_match(_peek(), TokenType::RBRACE)) {
                    // 针对情况{"a": true, "b": false,} 末尾多出逗号
                    throw std::invalid_argument(_buildErrMsg(
                        "json standard does not allow the use of trailing commas",
                        _peekPrev()));
                }
            } else if (!_isAtEnd() && !_match(_peek(), TokenType::COMMA)) {
                if (!_match(_peek(), TokenType::RBRACE)) {
                    // 对应情况{"a": true "b": false} 缺少逗号
                    // 或者情况{ "a": [1, { "b": 2 ], "c": 3 } 括号不匹配
                    const std::string curValMsg("'" + _peek()._value + "'" +
                                                " (at line " + std::to_string(_peek()._line + 1) + ")");
                    throw std::invalid_argument(_buildErrMsg(
                        "expected ',' or '}' here, but got a " + curValMsg,
                        _peekPrev(), _peekPrev()._value.size()));
                }
            } else {
                // 针对情况{"hello": null 未闭合状态
                throw std::invalid_argument(_buildErrMsg(
                    "json object not closed, expected ',' or '}'", _peekPrev(),
                    _peekPrev()._value.size()));
            }
        }

        return JsonValue(obj);
    }

    JsonValue Parser::_parseArray() {
        _advance();
        if (_isAtEnd()) {
            throw std::invalid_argument(_buildErrMsg(
                "expected json object <value> or ']', but now we meet EOF",
                _peekPrev()));
        }
        std::vector<JsonValue> arr;

        while (!_isAtEnd() && !_match(_peek(), TokenType::RBRACKET)) {
            const JsonValue val = _parseValue();
            arr.push_back(val);
            _advance();

            if (!_isAtEnd() && _match(_peek(), TokenType::COMMA)) {
                _advance();
                if (_isAtEnd()) {
                    throw std::invalid_argument(_buildErrMsg(
                        "expected <value> or replace the comma to ']'", _peekPrev(),
                        _peekPrev()._value.size()));
                }

                if (_match(_peek(), TokenType::RBRACKET)) {
                    throw std::invalid_argument(_buildErrMsg(
                        "json standard does not allow the use of trailing commas",
                        _peekPrev()));
                }
            } else if (!_isAtEnd() && !_match(_peek(), TokenType::COMMA)) {
                if (!_match(_peek(), TokenType::RBRACKET)) {
                    const std::string curValMsg("'" + _peek()._value + "'" +
                                                " (at line " + std::to_string(_peek()._line + 1) + ")");
                    throw std::invalid_argument(_buildErrMsg(
                        "expected ',' or ']' here, but got a " + curValMsg,
                        _peekPrev(), _peekPrev()._value.size()));
                }
            } else {
                throw std::invalid_argument(_buildErrMsg(
                    "json array not closed, expected ',' or ']'", _peekPrev(),
                    _peekPrev()._value.size()));
            }
        }

        return JsonValue(arr);
    }

    const Token &Parser::_peekPrev() const {
        if (_curIndex - 1 < _lexer.getTokens().size()) {
            // 因为是size_t类型，_curIndex - 1不可能为负数，不做<0的判断
            return _lexer.getTokens()[_curIndex - 1];
        }
        throw std::out_of_range("Parser::_peekPrev() -> _curIndex - 1 out of range");
    }


    const Token &Parser::_peek() const {
        if (_curIndex < _lexer.getTokens().size()) {
            return _lexer.getTokens()[_curIndex];
        }
        throw std::out_of_range("Parser::_peek() -> _curIndex out of range");
    }

    const Token &Parser::_peekNext() const {
        if (_curIndex + 1 < _lexer.getTokens().size()) {
            return _lexer.getTokens()[_curIndex + 1];
        }
        throw std::out_of_range("Parser::_peekNext() -> _curIndex + 1 out of range");
    }


    void Parser::_advance() {
        if (_curIndex < _lexer.getTokens().size()) {
            ++_curIndex; // 向前走一步

            if (_curIndex < _lexer.getTokens().size())
                _curToken = _lexer.getTokens()[_curIndex];
            return;
        }
        throw std::invalid_argument("Parser::_advance() -> _curIndex out of range");
    }

    bool Parser::_match(const Token &token, TokenType type) const noexcept {
        return _curToken.type() == type;
    }

    bool Parser::_isAtEnd() const noexcept {
        return _curIndex >= _lexer.getTokens().size();
    }


    std::string Parser::_buildErrMsg(std::string &&msg, const Token &highlightObj,
                                     size_t offset) const noexcept {
        const size_t line = highlightObj._line;
        const size_t highlightPos = highlightObj._column + offset; // 这里的偏移量主要针对缺少:或者,之类的错误的精准高亮

        const std::string rowNum = std::to_string(line + 1);
        std::string errMsg(rowNum + " | ");
        const size_t indent = errMsg.size(); // 前置行号信息缩进长度

        errMsg += _lexer.getInput()[line] + "\n" + std::string(rowNum.size() + 3, ' ');
        for (size_t i = 0; i < _lexer.getInput()[line].size() + indent; ++i) {
            if (i > highlightPos)
                break;
            errMsg += i >= highlightPos ? '^' : ' ';
        }
        errMsg += "  " + msg;

        return errMsg;
    }
}
