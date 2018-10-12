#ifndef CC__FUNCTION__H
#define CC__FUNCTION__H

#include <vector>
#include <memory>
#include "Statement.h"
#include "Variable.h"

class Visitor;

class Function {
public:
    std::string name;
    Type returnType;
    std::vector<VariablePtr> parameters;
    StatementBlock statements;

    bool isVariable(const std::string &name) const;
    int visit(Visitor *v);
};

typedef std::unique_ptr<Function> FunctionPtr;

#endif//CC__FUNCTION__H
