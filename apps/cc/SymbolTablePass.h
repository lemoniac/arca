#ifndef CC__SYMBOL_TABLE_PASS__H
#define CC__SYMBOL_TABLE_PASS__H

#include "Visitor.h"
#include "TranslationUnit.h"

class SymbolTablePass: public Visitor {
public:
    SymbolTablePass(TranslationUnit &unit): unit(unit)
    {
    }

    int visit(Function &f);
    int visit(StatementBlock &block);
    int visit(ReturnStatement &ret);
    int visit(FunctionCall &f);
    int visit(TranslationUnit &unit);

    TranslationUnit &unit;
};

#endif//CC__SYMBOL_TABLE_PASS__H
