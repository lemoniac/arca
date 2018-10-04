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

#endif//CC__EXPRESSION__H
