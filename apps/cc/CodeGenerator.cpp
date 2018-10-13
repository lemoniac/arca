#include <iostream>
#include "CodeGenerator.h"
#include "Function.h"
#include "TranslationUnit.h"
#include "Expression.h"

int CodeGenerator::visit(Function &f)
{
    isLeaf = true;
    returnSeen = false;
    usedRegisters = 0;
    usedRegisters[14] = usedRegisters[15] = 1; // SP, LR
    for(unsigned i = 0; i < f.parameters.size(); i++)
    {
        f.parameters[i]->reg = i + 1;
        usedRegisters[i + 1] = 1;
    }

    functionName = f.name;

    std::cout << f.name << ":" << std::endl;

    visit(*f.statements.get());

    if(!isLeaf || !returnSeen)
    {
        std::cout << f.name << "_epilogue:" << std::endl;
        std::cout << "    ret" << std::endl << std::endl;
    }

    return 0;
}

int CodeGenerator::visit(StatementBlock &block)
{
    Scope s = {block.symbolTable};
    scope.push_back(s);

    // assign registers to locals
    for(auto &l : block.locals)
    {
        int r = getFreeRegister();
        if(r < 0) return -1;
        l->reg = r;
        usedRegisters[r] = 1;
        if(l->valueSet)
        {
            std::cout << "    r" << r << " = " << l->value << std::endl;
        }
    }

    for(const auto &s : block.statements)
        s->visit(this);

    // unassign registers
    for(auto &l : block.locals)
        usedRegisters[l->reg] = 0;

    scope.pop_back();

    return 0;
}

int CodeGenerator::visit(ReturnStatement &ret)
{
    returnSeen = true;

    if(ret.returnValue)
    {
        ret.returnValue->visit(this);
        std::cout << "    r1 = " << res << std::endl;
    }

    if(isLeaf)
        std::cout << "    ret" << std::endl;
    else
        std::cout << "    jmp " << functionName << "_epilogue" << std::endl;

    return 0;
}

int CodeGenerator::visit(FunctionCall &f)
{
    int r = 1;
    for(const auto &arg : f.arguments)
    {
        std::cout << "    r" << r << " = 0" << std::endl;
    }
    std::cout << "    call " << f.function << std::endl;
}

int CodeGenerator::visit(TranslationUnit &unit)
{
    //std::cout << "jmp main" << std::endl;
    for(auto &g : unit.globals)
        std::cout << "int " << g.name << std::endl;
    for(auto &f : unit.functions)
        f->visit(this);
}

int CodeGenerator::getFreeRegister()
{
    for(unsigned i = 1; i < usedRegisters.size(); i++)
        if(!usedRegisters[i])
            return i;

    return -1;
}

int CodeGenerator::generateLabel()
{
    labelCounter++;
    return labelCounter;
}

int CodeGenerator::visit(Assignment &assignment)
{
    rdest = scope.back().symbols->find(assignment.dest)->variable->reg;
    assignment.expression->visit(this);

    return 0;
}

int CodeGenerator::visit(IntConstant &constant)
{
    res = std::to_string(constant.value);
    return 0;
}

int CodeGenerator::visit(IdentifierExpr &identifier)
{
    auto *s = scope.back().symbols->find(identifier.name);
    if(!s)
        throw std::runtime_error("undefined identifier");

    if(s->type == Type::Int)
        res = "r" + std::to_string(s->variable->reg);
    return 0;
}

int CodeGenerator::visit(BinaryOpExpr &op)
{
    op.left->visit(this);
    auto left = res;
    op.right->visit(this);
    auto right = res;

    std::cout << "    r" << rdest << " = " << left << " " << op.to_str() << " " << right << std::endl;
    return 0;
}

int CodeGenerator::visit(If &ifStatement)
{
    ifStatement.expression->visit(this);
    
    int label = generateLabel();

    std::cout << "    r0 = " << res << " - 0" << std::endl;
    std::cout << "    jmp.nz if" << label << std::endl;

    ifStatement.block->visit(this);

    std::cout << "if" << label << ":" << std::endl;

    return 0;
}

int CodeGenerator::visit(GotoStatement &gotoStatement)
{
    std::cout << "    jmp " << gotoStatement.label << std::endl;
    return 0;
}

int CodeGenerator::visit(LabelStatement &label)
{
    std::cout << label.label << ":" << std::endl;
    return 0;
}
