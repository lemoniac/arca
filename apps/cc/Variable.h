#ifndef CC__VARIABLE__H
#define CC__VARIABLE__H

#include <memory>
#include <string>
#include "Types.h"

class Expression;
typedef std::unique_ptr<Expression> ExpressionPtr;

struct Variable {
    DeclarationSpecifier declSpec;
    std::string name;
    unsigned elems;
    ExpressionPtr value;
    bool valueSet = false;
    unsigned reg;
    bool isGlobal = false;
    bool isSigned = true;
    bool used = false;
    bool referenced = false;

    bool isConstant() const;
    std::string getValue() const;
};

typedef std::unique_ptr<Variable> VariablePtr;

#endif//CC__VARIABLE__H
