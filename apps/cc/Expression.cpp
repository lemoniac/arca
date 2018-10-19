#include "Expression.h"
#include "Visitor.h"

int IntConstant::visit(Visitor *visitor) { visitor->visit(*this); }
int IdentifierExpr::visit(Visitor *visitor) { visitor->visit(*this); }
int ParentExpr::visit(Visitor *visitor) { visitor->visit(*this); }
int BinaryOpExpr::visit(Visitor *visitor) { visitor->visit(*this); }

ExpressionPtr BinaryOpExpr::simplify()
{
    ::simplify(left);
    ::simplify(right);

    const IntConstant *left = dynamic_cast<const IntConstant *>(this->left.get());
    const IntConstant *right = dynamic_cast<const IntConstant *>(this->right.get());

    if(left && right)
    {
        auto res = std::make_unique<IntConstant>();
        switch(op)
        {
            case Op::Add: res->value = left->value + right->value; break;
            case Op::Sub: res->value = left->value - right->value; break;
            case Op::Mul: res->value = left->value * right->value; break;
            case Op::Div: res->value = left->value / right->value; break;
            default: return nullptr;
        }
        return res;
    }

    return nullptr;
}

ExpressionPtr ParentExpr::simplify()
{
    ::simplify(expr);
    if(dynamic_cast<IntConstant *>(expr.get()) || dynamic_cast<IdentifierExpr *>(expr.get()))
        return std::move(expr);

    return 0;
}

void simplify(ExpressionPtr &expr)
{
    auto e = expr->simplify();
    if(e)
        expr = std::move(e);
}
