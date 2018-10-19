#ifndef CC__SIMPLIFY_EXPRESSIONS__H
#define CC__SIMPLIFY_EXPRESSIONS__H

#include "Visitor.h"

class SimplifyExpressions: public Visitor {
public:
    int visit(Function &f);
    int visit(StatementBlock &block);
    int visit(ReturnStatement &ret);
    int visit(If &ifStatement);
    int visit(While &statement);
    int visit(FunctionCall &f);
    int visit(TranslationUnit &unit);
    int visit(Assignment &assignment);
    int visit(GotoStatement &gotoStatement);
    int visit(LabelStatement &label);

    int visit(IntConstant &constant);
    int visit(IdentifierExpr &identifier);
    int visit(ParentExpr &expr);
    int visit(BinaryOpExpr &op);
};

#endif//CC__SIMPLIFY_EXPRESSIONS__H
