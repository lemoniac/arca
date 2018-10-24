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
        SymbolTable::Symbol s = {p->name, p->declSpec.type, p.get()};
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
        SymbolTable::Symbol s = {l->name, l->declSpec.type, l.get()};
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
    return ifStatement.block->visit(this);
}

int SymbolTablePass::visit(While &statement)
{
    statement.expression->visit(this);
    return statement.block->visit(this);
}

int SymbolTablePass::visit(ReturnStatement &ret)
{
    if(ret.returnValue)
        return ret.returnValue->visit(this);
    return 0;
}

int SymbolTablePass::visit(FunctionCall &f)
{
    IdentifierExpr *function = dynamic_cast<IdentifierExpr *>(f.function.get());

    auto symbol = f.parent->symbolTable->find(function->name);
    if(symbol == nullptr)
    {
        std::cerr << "error: undefined function '" << function->name << "'" << std::endl;
        return -1;
    }
    return 0;
}

int SymbolTablePass::visit(TranslationUnit &unit)
{
    symbols.push_back(&unit.symbolTable);
    for(auto &g : unit.globals)
    {
        SymbolTable::Symbol s = {g->name, g->declSpec.type, g.get()};
        unit.symbolTable.symbols.push_back(s);
    }
    for(auto &f : unit.functions)
        f->visit(this);
}

int SymbolTablePass::visit(Assignment &assignment)
{
    return assignment.expression->visit(this);
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

int SymbolTablePass::visit(AssignmentExpr &expr)
{
    if(expr.lhs->visit(this) < 0) return -1;
    return expr.lhs->visit(this);
}
