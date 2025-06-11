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
        _pushArr(JsonValue(val));
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

    void sJson::_pushArr(JsonValue &&val) const {
        if (_astType != JsonType::Array) {
            throw std::logic_error("sJson::_pushArr only supports an array");
        }

        // 接受push_back传进来的通用引用，需要用完美转发而不是std::move
        _astRoot->getArray().emplace_back(std::forward<JsonValue>(val));
    }

}
