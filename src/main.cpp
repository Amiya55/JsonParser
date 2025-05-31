#include <exception>
#include <iostream>
#include "iniParser.h"
#include "jsonParser.h"

std::string load_json() {
    std::fstream fs;
    fs.open("data.json", std::ios::in);
    if (!fs.is_open()) {
        throw std::runtime_error("cannot find json file! check the path");
    }

    std::string str;
    std::string tmp;
    while (getline(fs, tmp))
        str += tmp;
    fs.close();

    return str;
}

void read_from_json() {
    try {
        // std::string jsonStr = load_json();
        // JsonSyntaxCheker().syntax_check(jsonStr, JsonTypeName::JsonObject);

        JsonFile jf;
        jf.open_json("data.json");
    } catch (std::exception& e) {
        std::cout << e.what() << std::endl;
    }
}

int main() {
    read_from_json();
    return 0;
}