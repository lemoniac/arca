#include "Expression.h"
#include "SimplifyExpressions.h"
#include "TranslationUnit.h"
#include "Variable.h"

int SimplifyExpressions::visit(Function &f)
{
    f.statements->visit(this);

    return 0;
}

int SimplifyExpressions::visit(StatementBlock &block)
{
    for(auto &l : block.locals)
    {
        if(l->valueSet)
            simplify(l->value);
    }

    for(auto &s : block.statements)
        s->visit(this);

    return 0;
}

int SimplifyExpressions::visit(ReturnStatement &ret) { if(ret.returnValue) simplify(ret.returnValue); }

int SimplifyExpressions::visit(If &ifStatement)
{
    simplify(ifStatement.expression);
    ifStatement.block->visit(this);
    return 0;
}

int SimplifyExpressions::visit(While &statement)
{
    simplify(statement.expression);
    statement.block->visit(this);
    return 0;
}

int SimplifyExpressions::visit(For &statement)
{
    simplify(statement.clause1);
    simplify(statement.expression2);
    simplify(statement.expression3);
    statement.block->visit(this);
    return 0;
}

int SimplifyExpressions::visit(FunctionCall &f)
{
    for(auto &e : f.arguments)
        simplify(e);

    return 0;
}

int SimplifyExpressions::visit(TranslationUnit &unit)
{
    for(auto &g : unit.globals)
    {
        if(g->valueSet)
            simplify(g->value);
    }

    for(auto &f : unit.functions)
        f->visit(this);

    return 0;
}

int SimplifyExpressions::visit(GotoStatement &gotoStatement) { return 0; }
int SimplifyExpressions::visit(LabelStatement &label) { return 0; }

int SimplifyExpressions::visit(IntConstant &constant) { return 0; }
int SimplifyExpressions::visit(IdentifierExpr &identifier) { return 0; }

int SimplifyExpressions::visit(AssignmentExpr &expr)
{
    simplify(expr.lhs);
    simplify(expr.rhs);
    return 0;
}

int SimplifyExpressions::visit(ParentExpr &expr)
{
    expr.simplify();
    return 0;
}

int SimplifyExpressions::visit(BinaryOpExpr &op)
{
    simplify(op.left);
    simplify(op.right);

    return 0;
}

int SimplifyExpressions::visit(UnaryOpExpr &op) { simplify(op.expr); return 0; }

int SimplifyExpressions::visit(FunctionCallExpr &f)
{
    for(auto &arg : f.arguments)
        simplify(arg);

    return 0;
}
