#ifndef CC__STATEMENT__H
#define CC__STATEMENT__H

#include <vector>
#include <memory>
#include "Variable.h"

class Visitor;

class Expression;
typedef std::unique_ptr<Expression> ExpressionPtr;

class Statement {
public:
    ~Statement() { }

    virtual int visit(Visitor *visitor) = 0;
};

typedef std::unique_ptr<Statement> StatementPtr;

class StatementBlock : public Statement {
public:
    std::vector<Variable> locals;
    std::vector<StatementPtr> statements;

    int visit(Visitor *visitor) { return 0; }
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

    int visit(Visitor *visitor);
};

#endif//CC__STATEMENT__H
