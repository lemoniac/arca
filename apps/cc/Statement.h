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
};

typedef std::unique_ptr<StatementBlock> StatementBlockPtr;

class Assignment : public Statement {
public:
    enum class Kind { Assign, Add, Sub, Mul, Div, Mod, And, Or, Xor, Left, Right };

    ExpressionPtr expression;

    static const char *to_str(Kind kind);

    int visit(Visitor *visitor);
};

class FunctionCall : public Statement {
public:
    std::string function;
    std::vector<ExpressionPtr> arguments;

    int visit(Visitor *visitor);
};

class While : public Statement {
public:
    ExpressionPtr expression;
    StatementBlockPtr block;

    int visit(Visitor *visitor);
    void setParent(StatementBlock *parent);
};

class For : public Statement {
public:
    ExpressionPtr clause1;
    ExpressionPtr expression2;
    ExpressionPtr expression3;
    StatementBlockPtr block;

    int visit(Visitor *visitor);
    void setParent(StatementBlock *parent);
};

class If : public Statement {
public:
    ExpressionPtr expression;
    StatementBlockPtr block;
    StatementBlockPtr elseBlock;

    int visit(Visitor *visitor);
    void setParent(StatementBlock *parent);
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
    std::string label;

    int visit(Visitor *visitor);
};

#endif//CC__STATEMENT__H
