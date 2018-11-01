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
    virtual void setParent(StatementBlock *parent);

    StatementBlock *parent = nullptr;
};

typedef std::unique_ptr<Statement> StatementPtr;

class StatementBlock : public Statement {
public:
    StatementBlock(): symbolTable(new SymbolTable()) { }

    std::vector<VariablePtr> locals;
    std::vector<StatementPtr> statements;

    void add(StatementPtr statement);

    std::unique_ptr<SymbolTable> symbolTable;

    int visit(Visitor *visitor);
    void setParent(StatementBlock *parent);
};

typedef std::unique_ptr<StatementBlock> StatementBlockPtr;

class While : public Statement {
public:
    ExpressionPtr expression;
    StatementPtr statement;

    int visit(Visitor *visitor);
};

class For : public Statement {
public:
    ExpressionPtr clause1;
    ExpressionPtr expression2;
    ExpressionPtr expression3;
    StatementPtr statement;

    int visit(Visitor *visitor);
};

class If : public Statement {
public:
    ExpressionPtr expression;
    StatementPtr statement;
    StatementPtr elseStatement;

    int visit(Visitor *visitor);
};

class ReturnStatement: public Statement {
public:
    ExpressionPtr returnValue;

    int visit(Visitor *visitor);
};

class GotoStatement: public Statement {
public:
    GotoStatement(const std::string &label): label(label) { }

    std::string label;

    int visit(Visitor *visitor);
};

class LabelStatement: public Statement {
public:
    ExpressionPtr label;

    int visit(Visitor *visitor);
};

class AsmStatement: public Statement {
public:
    std::string statement;

    int visit(Visitor *visitor);
};

#endif//CC__STATEMENT__H
