#include <iostream>
#include "Expression.h"
#include "SymbolTablePass.h"
#include "SymbolTable.h"

#define VISIT(a) if(a->visit(this) < 0) return -1;

int SymbolTablePass::visit(Function &f)
{
    unit.symbolTable.addFunction(f.name, &f);

    if(f.statements)
    {
        for(auto &p : f.parameters)
            f.statements->symbolTable->add(p->name, p->declSpec.type, p.get());

        VISIT(f.statements)
    }

    return 0;
}

int SymbolTablePass::visit(StatementBlock &block)
{
    symbols.push_back(block.symbolTable.get());

    for(auto &l : block.locals)
    {
        if(l->valueSet)
            VISIT(l->value)

        block.symbolTable->add(l->name, l->declSpec.type, l.get());
    }

    for(auto &s : block.statements)
        VISIT(s)

    symbols.pop_back();

    return 0;
}

int SymbolTablePass::visit(If &ifStatement)
{
    VISIT(ifStatement.expression)
    return ifStatement.statement->visit(this);
}

int SymbolTablePass::visit(While &statement)
{
    VISIT(statement.expression);
    return statement.statement->visit(this);
}

int SymbolTablePass::visit(For &statement)
{
    VISIT(statement.clause1);
    VISIT(statement.expression2);
    VISIT(statement.expression3);
    return statement.statement->visit(this);
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
            VISIT(g->value);
    }

    for(auto &f : unit.functions)
        VISIT(f)

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
    VISIT(member.parent);

    return 0;
}

int SymbolTablePass::visit(SubscriptExpr &expr)
{
    VISIT(expr.lhs);

    int s = size(expr.lhs->type());
    if(s > 1)
    {
        auto mult = std::make_unique<BinaryOpExpr>(BinaryOpExpr::Op::Mul);
        mult->left = std::move(expr.rhs);
        mult->right = std::make_unique<IntConstant>(s);
        expr.rhs = std::move(mult);
    }

    return expr.rhs->visit(this);
}

int SymbolTablePass::visit(ParentExpr &expr)
{
    return expr.expr->visit(this);
}

int SymbolTablePass::visit(BinaryOpExpr &op)
{
    VISIT(op.left);
    return op.right->visit(this);
}

int SymbolTablePass::visit(UnaryOpExpr &op)
{
    VISIT(op.expr);

    if(op.op == UnaryOpExpr::Op::AddrOf)
    {
        IdentifierExpr *id = dynamic_cast<IdentifierExpr *>(op.expr.get());
        if(id)
        {
            auto symbol = symbols.back()->find(id->name);
            if(symbol->variable)
                symbol->variable->referenced = true;
        }
    }

    return 0;
}

int SymbolTablePass::visit(AssignmentExpr &expr)
{
    VISIT(expr.lhs);
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
        VISIT(arg);

        if(symbol->function->parameters[i]->declSpec.type != arg->type())
            std::cerr << "error: wrong type" << std::endl;

        i++;
    }

    return 0;
}
