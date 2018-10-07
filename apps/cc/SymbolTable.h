#ifndef ARCA__CC__SYMBOL_TABLE__H
#define ARCA__CC__SYMBOL_TABLE__H

#include <string>
#include <vector>
#include "Types.h"

class SymbolTable {
public:
    struct Symbol {
        std::string name;
        Type type;
    };

    std::vector<Symbol> symbols;
    SymbolTable *parent = nullptr;

    Symbol *find(const std::string &name)
    {
        for(auto &s : symbols)
            if(s.name == name)
                return &s;

        if(parent != nullptr)
            return parent->find(name);

        return nullptr;
    }

    bool isLocal(const std::string &name) const
    {
        for(auto &s : symbols)
            if(s.name == name)
                return true;

        return false;
    }
};

#endif//ARCA__CC__SYMBOL_TABLE__H
