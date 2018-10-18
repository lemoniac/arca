#include "Expression.h"
#include "Variable.h"

bool Variable::isConstant() const
{
    if(value)
    {
        IntConstant *n = dynamic_cast<IntConstant *>(value.get());
        return n != nullptr;
    }

    return false;
}

int Variable::getValue() const
{
    IntConstant *n = dynamic_cast<IntConstant *>(value.get());
    return n->value;
}

