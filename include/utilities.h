#ifndef UTILITIES_H
#define UTILITIES_H

#include <string>

// get rid of the pattern strings before the str or behind the str
void trim(std::string &str, const char *pattern = " \t\n\r");

#endif // UTILITIES_H
