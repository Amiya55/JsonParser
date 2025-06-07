#include <exception>
#include <iostream>
#include "iniParser.h"
#include "jsonParser.h"
#include "jsonTypes.h"
#include "utilities.h"

std::string load_json() {
    std::fstream fs;
    fs.open("data.json", std::ios::in);
    if (!fs.is_open()) {
        throw std::runtime_error("cannot find json file! check the path");
    }

    std::string str;
    std::string tmp;
    while (getline(fs, tmp))
        str += tmp + '\n';
    fs.close();

    return str;
}

void read_from_json() {
    try {
        // std::string jsonStr = load_json();
        // trim(jsonStr);
        // JsonSyntaxChecker().syntax_check(jsonStr, JsonValue::Type::Object);

        // JsonFile jf;
        // jf.open_json("data.json");

        std::string json("     null    , \n  null,  ggsh, null false");
        simpleJson::Lexer le(json);
        le.peekToken();
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
    }
}

int main() {
    read_from_json();
    return 0;
}
