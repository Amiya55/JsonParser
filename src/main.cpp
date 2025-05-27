#include <exception>
#include <iostream>
#include "iniParser.h"
#include "jsonParser.h"

void read_from_json() {
    try {
        JsonFile jf;
        jf.open_json("test.json");

    } catch (std::exception& e) {
        std::cout << e.what() << std::endl;
    }
}

int main() {
    read_from_json();
    return 0;
}