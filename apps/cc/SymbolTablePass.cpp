#include <iostream>
#include "Expression.h"
#include "SymbolTablePass.h"
#include "SymbolTable.h"

int SymbolTablePass::visit(Function &f)
{
    unit.symbolTable.add(f.name, Type::Function);

    if(f.statements)
    {
        for(auto &p : f.parameters)
            f.statements->symbolTable->add(p->name, p->declSpec.type, p.get());

        f.statements->visit(this);
    }

    return 0;
}

int SymbolTablePass::visit(StatementBlock &block)
{
    symbols.push_back(block.symbolTable.get());

    for(auto &l : block.locals)
    {
        if(l->valueSet)
            l->value->visit(this);
        block.symbolTable->add(l->name, l->declSpec.type, l.get());
    }

    for(auto &s : block.statements)
        s->visit(this);

    symbols.pop_back();

    return 0;
}

int SymbolTablePass::visit(If &ifStatement)
{
    ifStatement.expression->visit(this);
    return ifStatement.block->visit(this);
}

int SymbolTablePass::visit(While &statement)
{
    statement.expression->visit(this);
    return statement.block->visit(this);
}

int SymbolTablePass::visit(For &statement)
{
    statement.clause1->visit(this);
    statement.expression2->visit(this);
    statement.expression3->visit(this);
    return statement.block->visit(this);
}

int SymbolTablePass::visit(ReturnStatement &ret)
{
    if(ret.returnValue)
        return ret.returnValue->visit(this);
    return 0;
}

int SymbolTablePass::visit(TranslationUnit &unit)
{
    symbols.push_back(&unit.symbolTable);
    for(auto &g : unit.globals)
    {
        unit.symbolTable.add(g->name, g->declSpec.type, g.get());
        if(g->valueSet)
            g->value->visit(this);
    }

    for(auto &f : unit.functions)
        f->visit(this);
}

int SymbolTablePass::visit(GotoStatement &gotoStatement)
{
    return 0;
}

int SymbolTablePass::visit(LabelStatement &label)
{
    return 0;
}

int SymbolTablePass::visit(IntConstant &constant) { return 0; }

int SymbolTablePass::visit(StringLiteral &str)
{
    if(unit.strings.find(str.value) == unit.strings.end())
    {
        std::string label = "__anonstr_" + std::to_string(unit.strings.size());
        unit.strings[str.value] = label;
    }

    return 0;
}

int SymbolTablePass::visit(IdentifierExpr &identifier)
{
    auto symbol = symbols.back()->find(identifier.name);
    if(!symbol)
    {
        std::cerr << "error: undefined identifier '" << identifier.name << "'" << std::endl;
        return -1;
    }

    identifier.symbol = symbol;

    return 0;
}

int SymbolTablePass::visit(MemberExpr &member)
{
    if(member.parent->visit(this) < 0) return -1;

    return 0;
}

int SymbolTablePass::visit(SubscriptExpr &expr)
{
    expr.lhs->visit(this);
    return expr.rhs->visit(this);
}

int SymbolTablePass::visit(ParentExpr &expr)
{
    return expr.expr->visit(this);
}

int SymbolTablePass::visit(BinaryOpExpr &op)
{
    if(op.left->visit(this) < 0) return -1;
    return op.right->visit(this);
}

int SymbolTablePass::visit(UnaryOpExpr &op)
{
    return op.expr->visit(this);
}

int SymbolTablePass::visit(AssignmentExpr &expr)
{
    if(expr.lhs->visit(this) < 0) return -1;
    return expr.rhs->visit(this);
}

int SymbolTablePass::visit(FunctionCallExpr &f)
{
    IdentifierExpr *function = dynamic_cast<IdentifierExpr *>(f.function.get());

    auto symbol = symbols.back()->find(function->name);
    if(!symbol)
    {
        std::cerr << "error: undefined function '" << function->name << "'" << std::endl;
        return -1;
    }

    if(symbol->type != Type::Function)
    {
        std::cerr << "error: identifier '" << function->name << "' is not a function" << std::endl;
        return -1;
    }

    if(symbol->function->parameters.size() != f.arguments.size())
    {
        std::cerr << "error: invalid number of arguments: " << function->name << std::endl;
        return -1;
    }

    int i = 0;
    for(auto &arg: f.arguments)
    {
        if(arg->visit(this) < 0)
            return -1;

        if(symbol->function->parameters[i]->declSpec.type != arg->type())
            std::cerr << "error: wrong type" << std::endl;

        i++;
    }

    return 0;
}
