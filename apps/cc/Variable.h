#ifndef CC__VARIABLE__H
#define CC__VARIABLE__H

#include <string>
#include "Types.h"

struct Variable {
    Type type;
    bool is_pointer;
    std::string name;
    unsigned elems;
    int value;
    bool valueSet = false;
    unsigned reg;
};

#endif//CC__VARIABLE__H
