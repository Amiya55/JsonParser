#ifndef SJAPI_H
#define SJAPI_H
/* !NOTE
 * 本文件为simpleJson库的一些开放接口
*/
#include "jsonParser.h"
#include <string>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include "config.h"


namespace simpleJson {
    class jsonIterator {
    public:
        jsonIterator();

        ~jsonIterator();

    private:
        // JsonValue&
    };


    class sJson {
        std::shared_ptr<JsonValue> _astRoot;
        JsonType _astType;

    public:
        using iterator = jsonIterator;
        using const_iterator = const jsonIterator;

        iterator begin();

        iterator end();

        static sJson fromFile(const std::string &filePath);

        static sJson fromStr(const std::string &jsonStr);

        static sJson array();

        static sJson object();

        explicit sJson(sJson &) = delete;

        sJson &operator=(sJson &) = delete;

        sJson(sJson &&) = default;

        sJson &operator=(sJson &&) = default;

        ~sJson() = default;

        std::shared_ptr<JsonValue> getRoot() noexcept;

        template<typename T, typename std::enable_if<
            is_json_type<typename std::decay<T>::type>::value, int>::type = 0>
        void push_back(T &&val) {
            if (_astType != JsonType::Array) {
                throw std::logic_error("sJson::_pushArr only supports an array");
            }

            _astRoot->getArray().emplace_back(std::forward<JsonValue>(JsonValue(val)));
        }

        void push_back(const std::initializer_list<JsonValue> &val) const;

        // 向json对象添加键值对
        JsonValue &push_back(const std::pair<std::string, JsonValue> &val);

        JsonValue &operator[](size_t index);

        JsonValue &operator[](const std::string &key);

    private:
        explicit sJson(const std::string &jsonStr);
    };
}


#endif  // SJAPI_H
