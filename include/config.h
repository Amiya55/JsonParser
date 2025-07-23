#ifndef CONFIG_H
#define CONFIG_H

#include <unordered_map>
#include <type_traits>

namespace simpleJson {
#define ERR_TYPE_MISMATCH "The type mismatch occured "  // 错误提示：类型不匹配

#define POS_T unsigned long long  // 关于某个token定位的数据类型
#define LENGTH_T unsigned long long  // 某个Token的长度的类型(Token长度以字符数为准)

    // 只允许与json有关的数据类型
    class JsonValue;
    template<typename T>
    using enableIfJson =
    std::enable_if_t<std::is_same_v<std::decay_t<T>, std::unordered_map<std::string, JsonValue> > ||
                     std::is_same_v<std::decay_t<T>, std::vector<JsonValue> > ||
                     std::is_same_v<std::decay_t<T>, std::string> ||
                     std::is_same_v<std::decay_t<T>, const char *> ||
                     std::is_same_v<std::decay_t<T>, bool> ||
                     std::is_same_v<std::decay_t<T>, std::nullptr_t> ||
                     std::is_floating_point_v<T> ||
                     std::is_integral_v<T>>;

    // 只允许字符串，包括字符串字面量，字符和std::string
    template<typename T>
    using enableIfString = std::enable_if_t<std::is_constructible_v<std::string, T> >;
} // namespace simpleJson

#endif // CONFIG_H
