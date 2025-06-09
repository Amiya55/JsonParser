#include "utilities.h"

void trim(std::string &str, const char *pattern) {
    size_t begin = str.find_first_not_of(pattern);
    str.erase(0, begin);

    size_t end = str.find_last_not_of(pattern);
    str.erase(end + 1);
}

int toInt(const std::string &str) {

}

double toDouble(const std::string &str) {

}

std::string unicodeToUtf8(const std::string &str) {

}
