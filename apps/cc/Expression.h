#ifndef CC__EXPRESSION__H
#define CC__EXPRESSION__H

#include <memory>
#include <string>

class Visitor;

class Expression {
public:
    virtual int visit(Visitor *visitor) = 0;
};

typedef std::unique_ptr<Expression> ExpressionPtr;

class IntConstant : public Expression {
public:
    int value;

    int visit(Visitor *visitor);
};

class IdentifierExpr : public Expression {
public:
    std::string name;

    int visit(Visitor *visitor);
};

class BinaryOpExpr : public Expression {
public:
    enum class Op {Add, Sub, Mul, Div, And, Or, Xor};
    Op op;
    ExpressionPtr left;
    ExpressionPtr right;

    int visit(Visitor *visitor);

    void setOp(int c)
    {
        switch(c)
        {
            case '+': op = Op::Add; break;
            case '-': op = Op::Sub; break;
            case '*': op = Op::Mul; break;
            case '/': op = Op::Div; break;
            case '&': op = Op::And; break;
            case '|': op = Op::Or;  break;
            case '^': op = Op::Xor; break;
        }
    }

    const char *to_str() const
    {
        switch(op)
        {
            case Op::Add: return "+";
            case Op::Sub: return "-";
        }

        return "?";
    }
};

#endif//CC__EXPRESSION__H
