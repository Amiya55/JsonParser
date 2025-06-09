#ifndef UTILITIES_H
#define UTILITIES_H

#include <string>

// get rid of the pattern strings before the str or behind the str
void trim(std::string &str, const char *pattern = " \t\n\r");

// transform string to int
int toInt(const std::string &str);

// transform string to double
double toDouble(const std::string &str);

// transform Unicode to chinese
std::string unicodeToUtf8(const std::string &str);

#endif // UTILITIES_H
