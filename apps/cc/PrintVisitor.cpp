#include <iostream>
#include "PrintVisitor.h"
#include "Function.h"
#include "TranslationUnit.h"

int PrintVisitor::visit(Function &f)
{
    std::cout << "Function: " << f.name << std::endl;
    for(auto &p : f.parameters)
        std::cout << "    Parameter: " << p.name << std::endl;

    visit(f.statements);

    return 0;
}

int PrintVisitor::visit(StatementBlock &block)
{
    for(const auto &l : block.locals)
        std::cout << "    local " << int(l.type) << " " << l.name << std::endl;

    for(const auto &s : block.statements)
        s->visit(this);

    return 0;
}
int PrintVisitor::visit(ReturnStatement &ret)
{
    return 0;
}

int PrintVisitor::visit(FunctionCall &call)
{
    std::cout << "    " << call.function << "()" << std::endl;
}

int PrintVisitor::visit(TranslationUnit &unit)
{
    for(auto &g : unit.globals)
        std::cout << "    global " << int(g.type) << " " << g.name << std::endl;
    for(auto &f : unit.functions)
        f->visit(this);

    return 0;
}