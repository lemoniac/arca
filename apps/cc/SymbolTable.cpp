#include <iostream>
#include "Struct.h"
#include "SymbolTable.h"
#include "Variable.h"

int SymbolTable::Symbol::size() const
{
    switch(type)
    {
        case Type::Char: return 1;
        case Type::Int: return 4;
        case Type::Struct: return structInfo->size();
    }

    return 0;
}

SymbolTable::Symbol *SymbolTable::find(const std::string &name)
{
    for(auto &s : symbols)
        if(s.name == name)
            return &s;

    if(parent != nullptr)
        return parent->find(name);

    return nullptr;
}

bool SymbolTable::isLocal(const std::string &name) const
{
    for(auto &s : symbols)
        if(s.name == name)
            return true;

    return false;
}

bool SymbolTable::add(Symbol &&symbol)
{
    if(isLocal(symbol.name))
    {
        std::cerr << "error: redeclaration of " << symbol.name << std::endl;
        return false;
    }

    if(symbol.type == Type::Struct)
    {
        auto s = find(symbol.variable->declSpec.structName);
        if(!s)
        {
            std::cerr << "error: unknown struct " << symbol.variable->declSpec.structName << std::endl;
            return false;
        }
        symbol.structInfo = s->structInfo;
    }

    symbols.emplace_back(symbol);
    return true;
}
