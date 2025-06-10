#include "utilities.h"

void trim(std::string &str, const char *pattern) {
    const size_t begin = str.find_first_not_of(pattern);
    str.erase(0, begin);

    const size_t end = str.find_last_not_of(pattern);
    str.erase(end + 1);
}


std::string unicodeToUtf8(const std::string &str) {

}
