#ifndef CC__VARIABLE__H
#define CC__VARIABLE__H

#include <memory>
#include <string>
#include "Types.h"

class Expression;
typedef std::unique_ptr<Expression> ExpressionPtr;

struct Variable {
    Type type;
    bool is_pointer;
    std::string name;
    unsigned elems;
    ExpressionPtr value;
    bool valueSet = false;
    unsigned reg;
    bool isGlobal = false;
    bool isConst = false;

    bool isConstant() const;
    int getValue() const;
};

typedef std::unique_ptr<Variable> VariablePtr;

#endif//CC__VARIABLE__H
