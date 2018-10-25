#include "Statement.h"
#include "Expression.h"
#include "Visitor.h"

int IntConstant::visit(Visitor *visitor) { visitor->visit(*this); }
int IdentifierExpr::visit(Visitor *visitor) { visitor->visit(*this); }
int MemberExpr::visit(Visitor *visitor) { visitor->visit(*this); }
int ParentExpr::visit(Visitor *visitor) { visitor->visit(*this); }
int BinaryOpExpr::visit(Visitor *visitor) { visitor->visit(*this); }
int UnaryOpExpr::visit(Visitor *visitor) { visitor->visit(*this); }
int AssignmentExpr::visit(Visitor *visitor) { visitor->visit(*this); }


void BinaryOpExpr::setOp(int c)
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

const char *BinaryOpExpr::to_str() const
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
    else if(left && (op == Op::Add || op == Op::Mul))
    {
        std::swap(this->left, this->right);
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

ExpressionPtr UnaryOpExpr::simplify()
{
    ::simplify(expr);

    if(op == Op::SizeOf)
    {
        auto s = dynamic_cast<IdentifierExpr *>(expr.get());
        if(s)
        {
            auto i = std::make_unique<IntConstant>();
            i->value = s->symbol->size();
            return i;
        }
    }

    return 0;
}

ExpressionPtr AssignmentExpr::simplify()
{
    ::simplify(lhs);
    ::simplify(rhs);

    return 0;
}

void simplify(ExpressionPtr &expr)
{
    auto e = expr->simplify();
    if(e)
        expr = std::move(e);
}
