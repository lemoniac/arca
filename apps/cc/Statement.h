#ifndef CC__STATEMENT__H
#define CC__STATEMENT__H

#include <vector>
#include <memory>
#include "SymbolTable.h"
#include "Variable.h"

class Visitor;

class Expression;
typedef std::unique_ptr<Expression> ExpressionPtr;

class StatementBlock;

class Statement {
public:
    ~Statement() { }

    virtual int visit(Visitor *visitor) = 0;

    StatementBlock *parent = nullptr;
};

typedef std::unique_ptr<Statement> StatementPtr;

class StatementBlock : public Statement {
public:
    StatementBlock(): symbolTable(new SymbolTable()) { }

    std::vector<Variable> locals;
    std::vector<StatementPtr> statements;

    SymbolTable *symbolTable;

    int visit(Visitor *visitor);
};

class Assignment : public Statement {
public:
    std::string dest;
    ExpressionPtr expression;

    int visit(Visitor *visitor);
};

class FunctionCall : public Statement {
public:
    std::string function;
    std::vector<ExpressionPtr> arguments;

    int visit(Visitor *visitor);
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
