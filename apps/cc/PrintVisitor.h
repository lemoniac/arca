#ifndef CC__PRINT_VISITOR__H
#define CC__PRINT_VISITOR__H

#include "Visitor.h"

class PrintVisitor : public Visitor {
public:
    int visit(Function &f);
    int visit(StatementBlock &block);
    int visit(ReturnStatement &ret);
    int visit(FunctionCall &call);
    int visit(TranslationUnit &unit);
    int visit(Assignment &assignment);
};

#endif//CC__PRINT_VISITOR__H
