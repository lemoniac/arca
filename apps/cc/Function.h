#ifndef CC__FUNCTION__H
#define CC__FUNCTION__H

#include <vector>
#include "Statement.h"
#include "Variable.h"

class Visitor;

class Function {
public:
    std::string name;
    Type returnType;
    std::vector<Variable> parameters;
    StatementBlock statements;

    bool isVariable(const std::string &name) const;
    int visit(Visitor *v);
};

#endif//CC__FUNCTION__H
