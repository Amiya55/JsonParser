#include <exception>
#include <iostream>
#include <fstream>
#include "iniParser.h"
#include "jsonParser.h"
#include "jsonTypes.h"
#include "utilities.h"

std::string load_json() {
    std::fstream fs;
    fs.open("tmp.json", std::ios::in);
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

        // std::string json("     null    , \n  null,  ggsh, null false");
        // std::string json("     \"\\u4e00678\": \"   \\n\\\\t\"\n, \" hello,  ");
        // std::string json("    12e23,\n  \\m123, 123\n 12.23e3  343e23");
        std::string json(load_json());
        // std::string json("\n\"s\": \"\",\n \"ok\":\"1\"");
        // std::string json("{\n\"hello\": true,\n \"null\":[null, false,]\n}");
        simpleJson::Lexer le(json);
        // std::cout << json << std::endl;
        le.peekToken();
        // std::cout << le.getTokens().size() << std::endl;
        // for (int i = 0; i < le.getTokens().size(); i++) {
        //     std::cout << le.getTokens()[i]._value << std::endl;
        // }
        std::vector<std::string> ch;
        // for (auto& e : le.getTokens()) {
        //     std::cout << e._value << std::endl;
        //     ch.push_back(e._value);
        // }

        simpleJson::Parser pa(le);
        pa.parse();

    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;

    }
}

int main() {
    read_from_json();
    return 0;
}
