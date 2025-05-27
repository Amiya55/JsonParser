#ifndef COMMINC_H
#define COMMINC_H

#include <stdint.h>

// file opening mod: read, write, append
enum class Permission : uint8_t { RD = (1 << 2), WR = (1 << 1), AP = (1 << 0) };

// the typename of json
enum class JsonTypeName {
    JsonObject,
    JsonArray,
    JsonInt,
    JsonFloat,
    JsonString,
    JsonBool,
    JsonNull
};

#endif  // COMMINC_H