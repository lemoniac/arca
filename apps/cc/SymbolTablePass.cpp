#include <iostream>
#include "SymbolTablePass.h"
#include "SymbolTable.h"

int SymbolTablePass::visit(Function &f)
{
    SymbolTable::Symbol s = {f.name, Type::Function};
    unit.symbolTable.symbols.push_back(s);

    for(const auto &p : f.parameters)
    {
        SymbolTable::Symbol s = {p.name, p.type};
        f.statements.symbolTable->symbols.push_back(s);
    }

    f.statements.visit(this);

    return 0;
}

int SymbolTablePass::visit(StatementBlock &block)
{
    for(const auto &l : block.locals)
    {
        SymbolTable::Symbol s = {l.name, l.type};
        block.symbolTable->symbols.push_back(s);
    }

    for(auto &s : block.statements)
        s->visit(this);

    return 0;
}

int SymbolTablePass::visit(ReturnStatement &ret) { return 0; }

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
    for(auto &g : unit.globals)
    {
        SymbolTable::Symbol s = {g.name, g.type};
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
    return 0;
}
