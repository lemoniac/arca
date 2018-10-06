#include <iostream>
#include "CodeGenerator.h"
#include "Function.h"

int CodeGenerator::visit(Function &f)
{
    isLeaf = true;
    returnSeen = false;
    usedRegisters = 0;
    usedRegisters[14] = usedRegisters[15] = 1; // SP, LR
    for(unsigned i = 0; i < f.parameters.size(); i++)
        usedRegisters[i + 1] = 1;

    scope.functionName = f.name;

    std::cout << f.name << ":" << std::endl;

    visit(f.statements);

    if(!isLeaf || !returnSeen)
    {
        std::cout << f.name << "_epilogue:" << std::endl;
        std::cout << "    ret" << std::endl << std::endl;
    }

    return 0;
}

int CodeGenerator::visit(StatementBlock &block)
{
    // assign registers to locals
    for(auto &l : block.locals)
    {
        int r = getFreeRegister();
        if(r < 0) return -1;
        l.reg = r;
        usedRegisters[r] = 1;
    }

    for(const auto &s : block.statements)
        s->visit(this);

    // unassign registers
    for(auto &l : block.locals)
        usedRegisters[l.reg] = 0;

    return 0;
}

int CodeGenerator::visit(ReturnStatement &ret)
{
    returnSeen = true;

    if(isLeaf)
        std::cout << "    ret" << std::endl;
    else
        std::cout << "    jmp " << scope.functionName << "_epilogue" << std::endl;

    return 0;
}

int CodeGenerator::visit(FunctionCall &f)
{
    int r = 1;
    for(const auto &arg : f.arguments)
    {
        std::cout << "    r" << r << " = 0" << std::endl;
    }
    std::cout << "   call " << f.function << std::endl;
}

int CodeGenerator::getFreeRegister()
{
    for(unsigned i = 1; i < usedRegisters.size(); i++)
        if(!usedRegisters[i])
            return i;

    return -1;
}
