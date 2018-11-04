#ifndef CC__EXPRESSION__H
#define CC__EXPRESSION__H

#include <memory>
#include <string>
#include "Statement.h"

class Visitor;
class Symbol;
typedef std::shared_ptr<Symbol> SymbolPtr;

class Expression;
typedef std::unique_ptr<Expression> ExpressionPtr;

class Expression: public Statement {
public:
    virtual int visit(Visitor *visitor) = 0;
    virtual ExpressionPtr simplify() { return nullptr; }
    virtual Type type() const { return Type::Unknown; }
};

class IntConstant : public Expression {
public:
    int value;

    int visit(Visitor *visitor);
    Type type() const { return Type::Int; }
};

class StringLiteral: public Expression {
public:
    std::string value;

    int visit(Visitor *visitor);
    Type type() const { return Type::Char; }
};

class IdentifierExpr : public Expression {
public:
    IdentifierExpr(const std::string &name): name(name) { }

    std::string name;
    bool ref = false;
    SymbolPtr symbol = nullptr;

    int visit(Visitor *visitor);
    Type type() const { return symbol->type; }
};

class MemberExpr: public Expression {
public:
    std::string name;
    ExpressionPtr parent;

    int visit(Visitor *visitor);
};

class SubscriptExpr: public Expression {
public:
    ExpressionPtr lhs;
    ExpressionPtr rhs;

    int visit(Visitor *visitor);
};

class ParentExpr: public Expression {
public:
    ExpressionPtr expr;

    int visit(Visitor *visitor);
    ExpressionPtr simplify();

    Type type() const { return expr->type(); }
};

class BinaryOpExpr : public Expression {
public:
    enum class Op {Add, Sub, Mul, Div, Mod, And, Or, Xor, LShift, RShift, Eq, NEq, GT, LT, GE, LE};
    Op op;
    ExpressionPtr left;
    ExpressionPtr right;

    int visit(Visitor *visitor);

    void setOp(int c);
    const char *to_str() const;

    ExpressionPtr simplify();
    Type type() const;
};

class UnaryOpExpr: public Expression {
public:
    enum class Op {Neg, AddrOf, Ref, SizeOf, PreInc, PostInc, PreDec, PostDec};
    Op op;
    ExpressionPtr expr;

    int visit(Visitor *visitor);
    ExpressionPtr simplify();

    static Op from_token(int token);
};

class AssignmentExpr: public Expression {
public:
    enum class Kind { Assign, Add, Sub, Mul, Div, Mod, And, Or, Xor, Left, Right };

    ExpressionPtr lhs;
    ExpressionPtr rhs;
    int kind = 0;

    int visit(Visitor *visitor);
    ExpressionPtr simplify();

    static const char *to_str(Kind kind);
};

class FunctionCallExpr: public Expression {
public:
    ExpressionPtr function;
    std::vector<ExpressionPtr> arguments;

    int visit(Visitor *visitor);
};

void simplify(ExpressionPtr &expr);

#endif//CC__EXPRESSION__H
