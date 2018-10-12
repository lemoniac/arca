#ifndef CC__VARIABLE__H
#define CC__VARIABLE__H

#include <memory>
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

typedef std::unique_ptr<Variable> VariablePtr;

#endif//CC__VARIABLE__H
