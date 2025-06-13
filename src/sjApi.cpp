#include "sjApi.h"
#include <string>
#include <fstream>
#include <algorithm>

#if __cplusplus >= 201703L
#include <filesystem>
#endif

namespace simpleJson {
    // jsonIterator::jsonIterator() {
    //
    // }
    //
    //
    //
    // sJson::iterator sJson::begin() {
    //     if (_parser->getAst().getType() == JsonType::Array)
    //         return iterator();
    //     return ;
    // }

    sJson sJson::fromFile(const std::string &filePath) {
#if __cplusplus >= 201703L
        const std::filesystem::path path(filePath);
        if (!std::filesystem::exists(path)) {
            throw std::runtime_error("file does not exist");
        }

        std::string extension = path.extension().string();
        std::transform(extension.begin(), extension.end(), extension.begin(), tolower);
        if (extension != "json") {
            throw std::runtime_error("file extension is not supported, expected .json");
        }
#else  // __cplusplus >= 201703L
        std::string extension = filePath.substr(filePath.find_last_of('.') + 1);
        std::transform(extension.begin(), extension.end(), extension.begin(), tolower);
        if (extension != "json") {
            throw std::invalid_argument("file extension is not supported, expected .json");
        }
#endif  // __cplusplus >= 201703L

        std::fstream file(filePath, std::ios::in | std::ios::out);
        if (!file.is_open()) {
            throw std::runtime_error("could not open file -> " + filePath);
        }

        std::string jsonString;
        std::string line;
        while (std::getline(file, line)) {
            jsonString += line + "\n";
        }

        return sJson(jsonString);
    }

    sJson sJson::fromStr(const std::string &jsonStr) {
        return sJson(jsonStr);
    }

    sJson sJson::array() {
        return sJson("[]");
    }

    sJson sJson::object() {
        return sJson("{}");
    }

    void sJson::push_back(const std::initializer_list<JsonValue> &val) const {
        if (_astType != JsonType::Array) {
            throw std::logic_error("sJson::_pushArr only supports an array");
        }

        _astRoot->getArray().emplace_back(val);
    }

    JsonValue &sJson::push_back(const std::pair<std::string, JsonValue> &val) {
        if (_astType != JsonType::Object) {
            throw std::logic_error("sJson::_pushObj only supports an object ");
        }

        std::string jsonKey("\"" + val.first + "\"");
        if (_astRoot->getObject().find(jsonKey) != _astRoot->getObject().end()) {
            return _astRoot->getObject()[jsonKey];
        }

        _astRoot->getObject()[jsonKey] = val.second;
        return _astRoot->getObject()[jsonKey];
    }


    JsonValue &sJson::operator[](size_t index) {
        if (_astType != JsonType::Array) {
            throw std::logic_error("json is not an array, cannot use number index");
        }

        if (index >= _astRoot->getArray().size()) {
            throw std::out_of_range("index out of range");
        }

        return _astRoot->getArray()[index];
    }

    JsonValue &sJson::operator[](const std::string &key) {
        if (_astType != JsonType::Object) {
            throw std::logic_error("json is not an object , cannot use string key");
        }

        return push_back(std::make_pair<const std::string &, JsonValue>(key, nullptr));
    }

    sJson::sJson(const std::string &jsonStr)
        : _astRoot(nullptr)
          , _astType() {
        const Lexer lexer(jsonStr);
        Parser parser(lexer);
        parser.parse();

        _astRoot = parser.getAst();
        _astType = _astRoot->getType();
    }

    std::shared_ptr<JsonValue> sJson::getRoot() noexcept {
        return _astRoot;
    }
}
