#ifndef SJAPI_H
#define SJAPI_H
/* !NOTE
 * 本文件为simpleJson库的一些开放接口
*/
#include <string>

#include "jsonParser.h"

namespace simpleJson {
    class jsonIterator {
    public:
        jsonIterator();

        ~jsonIterator();

    private:
        // JsonValue&
    };


    class sJson {
        std::unique_ptr<Lexer> _lexer;
        std::unique_ptr<Parser> _parser;

    public:
        using iterator = jsonIterator;
        using const_iterator = const jsonIterator;

        iterator begin();

        iterator end();

        static sJson fromFile(const std::string &filePath);

        static sJson fromStr(const std::string &jsonStr);

        explicit sJson(sJson &) = delete;

        sJson &operator=(sJson &) = delete;

        sJson(sJson &&) = default;

        sJson &operator=(sJson &&) = default;

        ~sJson() = default;

    private:
        explicit sJson(const std::string &jsonStr);
    };

}


#endif  // SJAPI_H
