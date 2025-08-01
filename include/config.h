#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace simple_json
{
#ifndef ERR_TYPES
#define ERR_TYPES
#define ERR_TYPE_MISMATCH "The type mismatch occurred " // 错误提示：类型不匹配

#define ERR_MISSING_QUOTATION_MARK "Here missing quotation mark"           // 错误提示，缺少引号
#define ERR_INVALID_ESCAPE "Invalid escape sequence"                       // 错误提示，非法转义序列
#define ERR_INCOMPLETE_UNICODE_ESCAPE "Incomplete unicode escape sequence" // 错误提示，不完整的unicode转义序列
#define ERR_INVALID_UNICODE_ESCAPE "Invalid unicode escape sequence"       // 错误提示，非法的unicode转义序列

#define ERR_INCOMPLETE_NUMBER "Incomplete number literal" // 错误提示，不完整的数字
#define ERR_INVALID_NUMBER "Invalid number literal"       // 错误提示，非法的数字字面量

#define ERR_INVALID_LITERAL "Invalid json literal"     // 错误提示，非法的json字面量
#define LITERAL_GUESS_TRUE ", may be you mean true?"   // 字面量猜测，true
#define LITERAL_GUESS_FALSE ", may be you mean false?" // 字面量猜测，false
#define LITERAL_GUESS_NULL ", may be you mean null?"   // 字面量猜测，null

#define ERR_UNKNOWN_VALUE "Unknown json value" // 错误提示，无法识别的json数据类型

#define ERR_MISMATCH_TOP_LEVEL "Json top level should be object or array" // 错误提示，json顶层必须为对象或者数组

#define ERR_TYPE_NOT_OBJECT "It is not a json object" // 错误提示，类型与json对象不匹配
#define ERR_TYPE_NOT_ARRAY "It is not a json array"   // 错误提示，类型与json数组不匹配

#define ERR_EXPECTED_JSON_VALUE_TYPE "Expected a valid json value type here" // 错误提示，此处需要一个合法的json值类型
#define ERR_COLON_EXPECTED "Expected a colon after key"                      // 错误提示，此处需要一个冒号:
#define ERR_COMMA_EXPECTED "Expected a comma here"                           // 错误提示，此处需要一个逗号
#define ERR_TRAILING_COMMA "Trailing comma is not allowed in json"           // 错误提示，尾随逗号错误
#define ERR_OBJECT_NOT_CLOSED "json object not closed"                       // 错误提示，json对象未闭合
#define ERR_ARRAY_NOT_CLOSED "json array not closed"                         // 错误提示，json数组未闭合
#define ERR_OBJECT_KEY_MUST_BE_STRING "object key must be string"            // 错误提示，对象的键必须为字符串

#endif

// json扩展规则
#ifndef JSON_EXTENSION
#define ALLOW_TRAILING_COMMA false // 允许尾随逗号
#endif

#ifndef POS_T
#define POS_T unsigned long long // 关于某个token定位的数据类型
#endif

#ifndef LENGTH_T
#define LENGTH_T unsigned long long // 某个Token的长度的类型(Token长度以字符数为准)
#endif

// 只允许与json有关的数据类型
class JsonValue;
template <typename T>
using enableIfJson =
    std::enable_if_t<std::is_same_v<std::decay_t<T>, std::unordered_map<std::string, JsonValue>> ||
                     std::is_same_v<std::decay_t<T>, std::vector<JsonValue>> ||
                     std::is_same_v<std::decay_t<T>, std::string> || std::is_same_v<std::decay_t<T>, const char *> ||
                     std::is_same_v<std::decay_t<T>, bool> || std::is_same_v<std::decay_t<T>, std::nullptr_t> ||
                     std::is_floating_point_v<T> || std::is_integral_v<T>>;

// 只允许字符串，包括字符串字面量，字符和std::string
template <typename T> using enableIfString = std::enable_if_t<std::is_constructible_v<std::string, T>>;
} // namespace simple_json

#endif // CONFIG_H
