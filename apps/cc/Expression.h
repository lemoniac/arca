#ifndef CC__EXPRESSION__H
#define CC__EXPRESSION__H

class Expression {
};

typedef std::unique_ptr<Expression> ExpressionPtr;

class IntConstant : public Expression {
public:
    int value;
};

class IdentifierExpr : public Expression {
public:
    std::string name;
};

class BinaryOpExpr : public Expression {
public:
    enum class Op {Add, Sub, Mul, Div, And, Or, Xor};
    Op op;
    ExpressionPtr left;
    ExpressionPtr right;

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
};

#endif//CC__EXPRESSION__H
