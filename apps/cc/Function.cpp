#include "Function.h"
#include "Visitor.h"

bool Function::isVariable(const std::string &name) const
{
    for(const auto &p : parameters) if(p->name == name) return true;
    for(const auto &l : statements->locals) if(l->name == name) return true;

    return false;
}

int Function::visit(Visitor *v)
{
    v->visit(*this);
}
