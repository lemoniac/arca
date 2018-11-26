#include "Expression.h"
#include "Variable.h"

bool Variable::isConstant() const
{
    if(value)
    {
        IntConstant *n = dynamic_cast<IntConstant *>(value.get());
        if(n) return true;

        StringLiteral *s = dynamic_cast<StringLiteral *>(value.get());
        return s != nullptr;
    }

    return false;
}

std::string Variable::getValue() const
{
    IntConstant *n = dynamic_cast<IntConstant *>(value.get());
    if(n)
        return std::to_string(n->value);

    StringLiteral *s = dynamic_cast<StringLiteral *>(value.get());
    if(s)
        return s->value;

    InitializerListExpr *il = dynamic_cast<InitializerListExpr *>(value.get());
    if(il)
    {
        std::string res;
        for(auto &e : il->elements)
            res += " ???";
        return res;
    }

    return "???";
}

int Variable::getValueInt() const
{
    IntConstant *n = dynamic_cast<IntConstant *>(value.get());
    if(n)
        return n->value;

    return -1;
}
