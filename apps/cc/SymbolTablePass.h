#ifndef CC__SYMBOL_TABLE_PASS__H
#define CC__SYMBOL_TABLE_PASS__H

#include <vector>
#include "Visitor.h"
#include "TranslationUnit.h"

class SymbolTable;

class SymbolTablePass: public Visitor {
public:
    SymbolTablePass(TranslationUnit &unit): unit(unit)
    {
    }

    int visit(Function &f);
    int visit(StatementBlock &block);
    int visit(If &ifStatement);
    int visit(While &statement);
    int visit(ReturnStatement &ret);
    int visit(FunctionCall &f);
    int visit(TranslationUnit &unit);
    int visit(Assignment &assignment);
    int visit(GotoStatement &gotoStatement);
    int visit(LabelStatement &label);

    int visit(IntConstant &constant);
    int visit(IdentifierExpr &identifier);
    int visit(BinaryOpExpr &op);

    TranslationUnit &unit;

    std::vector<SymbolTable *> symbols;
};

#endif//CC__SYMBOL_TABLE_PASS__H
