#include <iostream>
#include "Expression.h"
#include "SymbolTablePass.h"
#include "SymbolTable.h"

int SymbolTablePass::visit(Function &f)
{
    SymbolTable::Symbol s = {f.name, Type::Function};
    unit.symbolTable.symbols.push_back(s);

    for(auto &p : f.parameters)
    {
        SymbolTable::Symbol s = {p->name, p->type, p.get()};
        f.statements->symbolTable->symbols.push_back(s);
    }

    f.statements->visit(this);

    return 0;
}

int SymbolTablePass::visit(StatementBlock &block)
{
    symbols.push_back(block.symbolTable);

    for(auto &l : block.locals)
    {
        SymbolTable::Symbol s = {l->name, l->type, l.get()};
        block.symbolTable->symbols.push_back(s);
    }

    for(auto &s : block.statements)
        s->visit(this);

    symbols.pop_back();

    return 0;
}

int SymbolTablePass::visit(If &ifStatement)
{
    ifStatement.expression->visit(this);
    ifStatement.block->visit(this);
}

int SymbolTablePass::visit(ReturnStatement &ret)
{
    if(ret.returnValue)
        return ret.returnValue->visit(this);
    return 0;
}

int SymbolTablePass::visit(FunctionCall &f)
{
    auto symbol = f.parent->symbolTable->find(f.function);
    if(symbol == nullptr)
    {
        std::cerr << "error: undefined function '" << f.function << "'" << std::endl;
        return -1;
    }
    return 0;
}

int SymbolTablePass::visit(TranslationUnit &unit)
{
    symbols.push_back(&unit.symbolTable);
    for(auto &g : unit.globals)
    {
        SymbolTable::Symbol s = {g.name, g.type, &g};
        unit.symbolTable.symbols.push_back(s);
    }
    for(auto &f : unit.functions)
        f->visit(this);
}

int SymbolTablePass::visit(Assignment &assignment)
{
    auto *dst = assignment.parent->symbolTable->find(assignment.dest);
    if(dst == nullptr)
    {
        std::cerr << "error: undefined identifier '" << assignment.dest << "'" << std::endl;
        return -1;
    }

    if(assignment.expression->visit(this) < 0)
        return -1;

    return 0;
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
    auto *dst = symbols.back()->find(identifier.name);
    if(dst == nullptr)
    {
        std::cerr << "error: undefined identifier '" << identifier.name << "'" << std::endl;
        return -1;
    }

    return 0;
}

int SymbolTablePass::visit(BinaryOpExpr &op)
{
    op.left->visit(this);
    op.right->visit(this);

    return 0;
}
