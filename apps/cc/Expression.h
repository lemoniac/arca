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
    virtual ExpressionPtr symplify() { return nullptr; };
};

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

class ParentExpr: public Expression {
public:
    ExpressionPtr expr;

    int visit(Visitor *visitor);
};

class BinaryOpExpr : public Expression {
public:
    enum class Op {Add, Sub, Mul, Div, Mod, And, Or, Xor, LShift, RShift, Eq, NEq, GT, LT, GE, LE};
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
            case '%': op = Op::Mod; break;
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
            case Op::Mul: return "*";
            case Op::Div: return "/";
            case Op::Mod: return "%";
            case Op::And: return "&";
            case Op::Or:  return "|";
            case Op::Xor: return "^";
            case Op::LShift: return "<<";
            case Op::RShift: return ">>";
            case Op::Eq: return "^";
            case Op::NEq: return "^";
            case Op::LT: return "<";
            case Op::GT: return "-";
            case Op::LE: return "-";
            case Op::GE: return "-";
        }

        return "?";
    }

    ExpressionPtr symplify();
};

#endif//CC__EXPRESSION__H
