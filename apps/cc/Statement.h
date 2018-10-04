#ifndef CC__STATEMENT__H
#define CC__STATEMENT__H

#include <vector>
#include <memory>
#include "Variable.h"

class Expression;
typedef std::unique_ptr<Expression> ExpressionPtr;

class Statement {
};

class StatementBlock : public Statement {
public:
    std::vector<Variable> locals;
    std::vector<Statement> statements;
};

class Assignment : public Statement {
public:
    std::string dest;
    ExpressionPtr expression;
};

class FunctionCall : public Statement {
public:
    std::string function;
    std::vector<ExpressionPtr> arguments;
};

class While {
public:
    ExpressionPtr expression;
    StatementBlock block;
};

class ReturnStatement: public Statement {
public:
    ExpressionPtr returnValue;
};

#endif//CC__STATEMENT__H
