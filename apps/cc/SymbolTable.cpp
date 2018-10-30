#include <iostream>
#include "Struct.h"
#include "SymbolTable.h"
#include "Variable.h"

int Symbol::size() const
{
    switch(type)
    {
        case Type::Char: return 1;
        case Type::Int: return 4;
        case Type::Struct: return structInfo->size();
    }

    return 0;
}

Symbol::Symbol(const std::string &name, Type type, Variable *variable, StructPtr structInfo):
    name(name), type(type), variable(variable), structInfo(structInfo)
{
}


SymbolPtr SymbolTable::find(const std::string &name)
{
    for(auto &s : symbols)
        if(s->name == name)
            return s;

    if(parent != nullptr)
        return parent->find(name);

    return nullptr;
}

bool SymbolTable::isLocal(const std::string &name) const
{
    for(auto &s : symbols)
        if(s->name == name)
            return true;

    return false;
}

bool SymbolTable::add(const std::string &name, Type type, Variable *var, StructPtr structInfo)
{
    if(isLocal(name))
    {
        std::cerr << "error: redeclaration of " << name << std::endl;
        return false;
    }

    auto symbol = std::make_shared<Symbol>(name, type, var, structInfo);

    if(type == Type::Struct)
    {
        auto s = find(var->declSpec.structName);
        if(!s)
        {
            std::cerr << "error: unknown struct " << var->declSpec.structName << std::endl;
            return false;
        }
        symbol->structInfo = s->structInfo;
    }

    symbols.push_back(symbol);
    return true;
}
