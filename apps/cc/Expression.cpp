#include "Expression.h"
#include "Visitor.h"

int IntConstant::visit(Visitor *visitor) { visitor->visit(*this); }
int IdentifierExpr::visit(Visitor *visitor) { visitor->visit(*this); }
int BinaryOpExpr::visit(Visitor *visitor) { visitor->visit(*this); }

ExpressionPtr BinaryOpExpr::symplify()
{
    const IntConstant *left = dynamic_cast<const IntConstant *>(this->left.get());
    const IntConstant *right = dynamic_cast<const IntConstant *>(this->right.get());

    if(left && right)
    {
        auto res = std::make_unique<IntConstant>();
        switch(op)
        {
            case Op::Add: res->value = left->value + right->value; break;
            case Op::Sub: res->value = left->value - right->value; break;
            default: return nullptr;
        }
        return res;
    }

    return nullptr;
}
