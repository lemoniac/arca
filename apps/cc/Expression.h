#ifndef CC__EXPRESSION__H
#define CC__EXPRESSION__H

#include <memory>
#include <string>

class Visitor;

class Expression;
typedef std::unique_ptr<Expression> ExpressionPtr;

class Expression {
public:
    virtual int visit(Visitor *visitor) = 0;
    virtual ExpressionPtr simplify() { return nullptr; }
};

class IntConstant : public Expression {
public:
    int value;

    int visit(Visitor *visitor);
};

class IdentifierExpr : public Expression {
public:
    IdentifierExpr(const std::string &name): name(name) { }
    std::string name;
    bool ref = false;

    int visit(Visitor *visitor);
};

class MemberExpr: public Expression {
public:
    std::string name;
    ExpressionPtr parent;

    int visit(Visitor *visitor);
};

class ParentExpr: public Expression {
public:
    ExpressionPtr expr;

    int visit(Visitor *visitor);
    ExpressionPtr simplify();
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
};

class AssignmentExpr: public Expression {
public:
    ExpressionPtr lhs;
    ExpressionPtr rhs;
    int kind = 0;

    int visit(Visitor *visitor);
    ExpressionPtr simplify();
};

void simplify(ExpressionPtr &expr);

#endif//CC__EXPRESSION__H
