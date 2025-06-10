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

    sJson::sJson(const std::string &jsonStr)
        : _lexer(new Lexer(jsonStr))
          , _parser(new Parser(*_lexer)) {
        _parser->parse();
    }
}
