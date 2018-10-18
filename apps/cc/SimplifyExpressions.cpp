#include "Expression.h"
#include "SimplifyExpressions.h"
#include "TranslationUnit.h"
#include "Variable.h"

int SimplifyExpressions::visit(Function &f)
{
    return 0;
}

int SimplifyExpressions::visit(StatementBlock &block) { return 0; }
int SimplifyExpressions::visit(ReturnStatement &ret) { return 0; }
int SimplifyExpressions::visit(If &ifStatement) { return 0; }
int SimplifyExpressions::visit(While &statement) { return 0; }
int SimplifyExpressions::visit(FunctionCall &f) { return 0; }

int SimplifyExpressions::visit(TranslationUnit &unit)
{
    for(auto &g : unit.globals)
    {
        if(g->valueSet)
        {
            auto e = g->value->symplify();
            if(e)
                g->value = std::move(e);
        }
    }
    return 0;
}

int SimplifyExpressions::visit(Assignment &assignment) { return 0; }
int SimplifyExpressions::visit(GotoStatement &gotoStatement) { return 0; }
int SimplifyExpressions::visit(LabelStatement &label) { return 0; }

int SimplifyExpressions::visit(IntConstant &constant) { return 0; }
int SimplifyExpressions::visit(IdentifierExpr &identifier) { return 0; }
int SimplifyExpressions::visit(BinaryOpExpr &op) { return 0; }
