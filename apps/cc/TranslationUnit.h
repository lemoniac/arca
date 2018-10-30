#ifndef CC__TRANSLATION_UNIT__H
#define CC__TRANSLATION_UNIT__H

#include <map>
#include <vector>
#include "Function.h"
#include "Variable.h"
#include "SymbolTable.h"

class TranslationUnit {
public:
    std::vector<VariablePtr> globals;
    std::vector<FunctionPtr> functions;

    SymbolTable symbolTable;

    std::map<std::string, std::string> strings;
};

#endif//CC__TRANSLATION_UNIT__H
