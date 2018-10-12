#ifndef CC__PRINT_VISITOR__H
#define CC__PRINT_VISITOR__H

#include "Visitor.h"

class PrintVisitor : public Visitor {
public:
    int visit(Function &f);
    int visit(StatementBlock &block);
    int visit(If &ifStatement);
    int visit(ReturnStatement &ret);
    int visit(FunctionCall &call);
    int visit(TranslationUnit &unit);
    int visit(Assignment &assignment);
    int visit(GotoStatement &gotoStatement);
    int visit(LabelStatement &label);

    int visit(IntConstant &constant);
    int visit(IdentifierExpr &identifier);
    int visit(BinaryOpExpr &op);
};

#endif//CC__PRINT_VISITOR__H
