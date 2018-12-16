#include "Statement.h"
#include "Expression.h"
#include "Visitor.h"
#include "Tokens.h"

int IntConstant::visit(Visitor *visitor) { visitor->visit(*this); }
int StringLiteral::visit(Visitor *visitor) { visitor->visit(*this); }
int IdentifierExpr::visit(Visitor *visitor) { visitor->visit(*this); }
int MemberExpr::visit(Visitor *visitor) { visitor->visit(*this); }
int SubscriptExpr::visit(Visitor *visitor) { visitor->visit(*this); }
int ParentExpr::visit(Visitor *visitor) { visitor->visit(*this); }
int BinaryOpExpr::visit(Visitor *visitor) { visitor->visit(*this); }
int UnaryOpExpr::visit(Visitor *visitor) { visitor->visit(*this); }
int AssignmentExpr::visit(Visitor *visitor) { visitor->visit(*this); }
int FunctionCallExpr::visit(Visitor *visitor) { visitor->visit(*this); }
int InitializerListExpr::visit(Visitor *visitor) { visitor->visit(*this); }


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
        case Op::NEq: return "|";
        case Op::LT: return "<";
        case Op::GT: return ">";
        case Op::LE: return "<=";
        case Op::GE: return ">=";
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
        int value;
        switch(op)
        {
            case Op::Add: value = left->value + right->value; break;
            case Op::Sub: value = left->value - right->value; break;
            case Op::Mul: value = left->value * right->value; break;
            case Op::Div: value = left->value / right->value; break;

            case Op::And: value = left->value & right->value; break;
            case Op::Or:  value = left->value | right->value; break;
            case Op::Xor: value = left->value ^ right->value; break;

            case Op::LShift: value = left->value << right->value; break;
            case Op::RShift: value = left->value >> right->value; break;

            default: return nullptr;
        }
        return std::make_unique<IntConstant>(value);
    }
    else if(left && (op == Op::Add || op == Op::Mul || op == Op::Or || op == Op::And))
    {
        std::swap(this->left, this->right);
    }
    else if(op == BinaryOpExpr::Op::GT)
    {
        op = BinaryOpExpr::Op::LT;
        std::swap(this->left, this->right);
    }

    return nullptr;
}

Type BinaryOpExpr::type() const
{
    if(left->type() == right->type())
        return left->type();

    return Type::Unknown;
}

ExpressionPtr IdentifierExpr::simplify()
{
    if(symbol->variable && symbol->variable->declSpec.isConst)
    {
        if(symbol->type == Type::Int)
            return std::make_unique<IntConstant>(symbol->variable->getValueInt());
    }

    return 0;
}

ExpressionPtr MemberExpr::simplify()
{
    ::simplify(parent);

    return 0;
}

ExpressionPtr SubscriptExpr::simplify()
{
    ::simplify(lhs);
    ::simplify(rhs);

    return 0;
}

ExpressionPtr ParentExpr::simplify()
{
    ::simplify(expr);
    return std::move(expr);
}

ExpressionPtr UnaryOpExpr::simplify()
{
    ::simplify(expr);

    if(op == Op::SizeOf)
    {
        auto s = dynamic_cast<IdentifierExpr *>(expr.get());
        if(s)
            return std::make_unique<IntConstant>(s->symbol->size());
    }

    return 0;
}

UnaryOpExpr::Op UnaryOpExpr::from_token(int token)
{
    switch(token)
    {
        case '-': return Op::Neg;
        case '&': return Op::AddrOf;
        case '*': return Op::Ref;
        case '!': return Op::Not;
        case INC_OP: return Op::PreInc;
        case DEC_OP: return Op::PreDec;
    }

    return Op::SizeOf;
}


ExpressionPtr AssignmentExpr::simplify()
{
    ::simplify(lhs);
    ::simplify(rhs);

    return 0;
}

const char *AssignmentExpr::to_str(AssignmentExpr::Kind kind)
{
    switch(kind)
    {
        case Kind::Assign: return "=";
        case Kind::Add: return "+";
        case Kind::Sub: return "-";
        case Kind::Mul: return "*";
        case Kind::Div: return "/";
        case Kind::And: return "&";
        case Kind::Or: return "|";
    }

    return "?";
}

void simplify(ExpressionPtr &expr)
{
    auto e = expr->simplify();
    if(e)
        expr = std::move(e);
}
