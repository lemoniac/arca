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

    return "???";
}

