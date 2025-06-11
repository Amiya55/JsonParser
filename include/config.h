#ifndef CONFIG_H
#define CONFIG_H
#include <type_traits>
#include <string>
#include <vector>
#include <unordered_map>
#include <initializer_list>

namespace simpleJson {
    class JsonValue;

    template<typename T>
    struct is_json_type : std::false_type {
    };

    template<>
    struct is_json_type<int> : std::true_type {
    };

    template<>
    struct is_json_type<long> : std::true_type {
    };

    template<>
    struct is_json_type<long long> : std::true_type {
    };

    template<>
    struct is_json_type<float> : std::true_type {
    };

    template<>
    struct is_json_type<double> : std::true_type {
    };

    template<>
    struct is_json_type<bool> : std::true_type {
    };

    template<>
    struct is_json_type<std::nullptr_t> : std::true_type {
    };

    template<>
    struct is_json_type<const char *> : std::true_type {
    };

    template<>
    struct is_json_type<std::string> : std::true_type {
    };

    template<>
    struct is_json_type<std::vector<JsonValue> > : std::true_type {
    };

    template<>
    struct is_json_type<std::unordered_map<std::string, JsonValue> > : std::true_type {
    };

    template<>
    struct is_json_type<std::initializer_list<JsonValue> > : std::true_type {
    };

}


#endif  // CONFIG_H
