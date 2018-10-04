#ifndef CC__VARIABLE__H
#define CC__VARIABLE__H

#include <string>

enum class Type { Error = -1, Void, Int };

struct Variable {
    Type type;
    bool is_pointer;
    std::string name;
    unsigned elems;
    int value;
    unsigned reg;
};

#endif//CC__VARIABLE__H
