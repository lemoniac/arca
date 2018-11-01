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

    Symbol(const std::string &name, Type type, Variable *variable, StructPtr structInfo, Function *function);

    int size() const;
};

typedef std::shared_ptr<Symbol> SymbolPtr;

class SymbolTable {
public:
    std::vector<SymbolPtr> symbols;
    SymbolTable *parent = nullptr;

    SymbolPtr find(const std::string &name);
    bool isLocal(const std::string &name) const;
    bool add(const std::string &name, Type type, Variable *var = nullptr, StructPtr structInfo = nullptr, Function *function = nullptr);
    bool addFunction(const std::string &name, Function *function = nullptr);
};

#endif//ARCA__CC__SYMBOL_TABLE__H
