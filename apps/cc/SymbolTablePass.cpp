#include <iostream>
#include "Expression.h"
#include "SymbolTablePass.h"
#include "SymbolTable.h"

int SymbolTablePass::visit(Function &f)
{
    Symbol s = {f.name, Type::Function};
    unit.symbolTable.symbols.push_back(s);

    for(auto &p : f.parameters)
    {
        Symbol s = {p->name, p->declSpec.type, p.get()};
        f.statements->symbolTable->symbols.push_back(s);
    }

    f.statements->visit(this);

    return 0;
}

int SymbolTablePass::visit(StatementBlock &block)
{
    symbols.push_back(block.symbolTable.get());

    for(auto &l : block.locals)
    {
        if(l->valueSet)
            l->value->visit(this);
        Symbol s = {l->name, l->declSpec.type, l.get()};
        block.symbolTable->add(std::move(s));
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
        Symbol s = {g->name, g->declSpec.type, g.get()};
        unit.symbolTable.add(std::move(s));
    }
    for(auto &f : unit.functions)
    {
        Symbol s = {f->name, Type::Function, 0};
        s.function = f.get();
        unit.symbolTable.add(std::move(s));
        f->visit(this);
    }
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
