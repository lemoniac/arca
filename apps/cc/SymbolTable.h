#ifndef ARCA__CC__SYMBOL_TABLE__H
#define ARCA__CC__SYMBOL_TABLE__H

#include <memory>
#include <string>
#include <vector>
#include "Types.h"

class Variable;
class Struct;
typedef std::shared_ptr<Struct> StructPtr;
class Function;

struct Symbol {
    std::string name;
    Type type;
    Variable *variable = nullptr;
    StructPtr structInfo;
    Function *function = nullptr;

    int size() const;
};

class SymbolTable {
public:
    std::vector<Symbol> symbols;
    SymbolTable *parent = nullptr;

    Symbol *find(const std::string &name);
    bool isLocal(const std::string &name) const;
    bool add(Symbol &&symbol);
};

#endif//ARCA__CC__SYMBOL_TABLE__H
