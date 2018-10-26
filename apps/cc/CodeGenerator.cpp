#include <iostream>
#include "CodeGenerator.h"
#include "Function.h"
#include "TranslationUnit.h"
#include "Expression.h"
#include "Struct.h"

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
    Scope s = {block.symbolTable.get()};
    scope.push_back(s);

    // assign registers to locals
    for(auto &l : block.locals)
    {
        int r = getFreeRegister();
        if(r < 0) return -1;
        l->reg = r;
        usedRegisters[r] = 1;

        // reserve space on the stack
        if(l->declSpec.type == Type::Struct || l->referenced)
        {
            auto s = scope.back().symbols->find(l->name);
            std::cout << "    r14 = r14 - " << s->size() << std::endl;
            std::cout << "    r" << r << " = r14" << std::endl;
        }

        if(l->valueSet)
        {
            rdest = r;
            l->value->visit(this);
            std::cout << "    r" << r << " = " << res << std::endl;
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
    IdentifierExpr *function = dynamic_cast<IdentifierExpr *>(f.function.get());
    std::cout << "    call " << function->name << std::endl;
}

int CodeGenerator::visit(TranslationUnit &unit)
{
    Scope s = {&unit.symbolTable};
    scope.push_back(s);

    bool initializeGlobals = false;
    for(const auto &g : unit.globals)
    {
        if(g->valueSet)
        {
            initializeGlobals = true;
            break;
        }
    }

    std::cout << "    r14 = 4092" << std::endl;
    if(initializeGlobals)
        std::cout << "jmp start" << std::endl;

    for(auto &s : unit.symbolTable.symbols)
    {
        if(s.type == Type::Struct && s.variable == nullptr)
        {
            std::cout << ".struct " << s.name << std::endl;
            for(const auto &m : s.structInfo->member)
                std::cout << "    int " << m.name << std::endl;
            std::cout << ".endstruct" << std::endl;
            std::cout << std::endl;
        }
    }

    for(auto &g : unit.globals)
    {
        if(g->declSpec.type == Type::Struct)
        {
            std::cout << "    struct " << g->declSpec.structName << " " << g->name << std::endl;
        }
        else
        {
            std::cout << "    int " << g->name;
            if(g->valueSet && g->isConstant())
                std::cout << " = " << g->getValue();
            else
                std::cout << " = 0";
            std::cout << std::endl;
        }
    }

    if(initializeGlobals)
    {
        std::cout << "start:" << std::endl;
        for(auto &g : unit.globals)
        {
            if(g->valueSet && !g->isConstant())
            {
                int r = getFreeRegister();
                rdest = r;
                g->value->visit(this);
                std::cout << "    *" << g->name << " = r" << r << std::endl;
            }
        }
    }

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
    std::bitset<32> oldRegisters = usedRegisters;
    assignment.expression->visit(this);
    usedRegisters = oldRegisters;
    return 0;
}

int CodeGenerator::visit(IntConstant &constant)
{
    res = std::to_string(constant.value);
    return 0;
}

int CodeGenerator::visit(IdentifierExpr &identifier)
{
    if(!identifier.symbol)
        throw std::runtime_error("undefined identifier: " + identifier.name);

    auto &s = identifier.symbol;

    if(s->variable->isGlobal)
    {
        int r = getFreeRegister();
        usedRegisters[r] = true;
        s->variable->reg = r;
        res = "r" + std::to_string(r);
        std::cout << "    " << res << " = " << (identifier.ref?"&":"*") << s->name << std::endl;
    }
    else if(s->type == Type::Int)
        res = "r" + std::to_string(s->variable->reg);

    return 0;
}

int CodeGenerator::visit(MemberExpr &member)
{
    member.parent->visit(this);

    IdentifierExpr *identifier = dynamic_cast<IdentifierExpr *>(member.parent.get());
    const auto symbol = scope.back().symbols->find(identifier->name);

    res = "*" + res + "." + symbol->variable->declSpec.structName + "." + member.name;

    return 0;
}

int CodeGenerator::visit(ParentExpr &expr)
{
    return expr.expr->visit(this);
}

int CodeGenerator::visit(BinaryOpExpr &op)
{
    int r = getFreeRegister();
    usedRegisters[r] = 1;
    op.left->visit(this);
    auto left = res;
    op.right->visit(this);
    auto right = res;

    std::cout << "    r" << r << " = " << left << " " << op.to_str() << " " << right << std::endl;
    res = "r" + std::to_string(r);
    return 0;
}

int CodeGenerator::visit(UnaryOpExpr &op)
{
    op.expr->visit(this);

    switch(op.op)
    {
        case UnaryOpExpr::Op::PreInc:
            std::cout << "    " << res << " = " << res << " + 1" << std::endl;
            break;

        case UnaryOpExpr::Op::PreDec:
            std::cout << "    " << res << " = " << res << " - 1" << std::endl;
            break;
    }

    return 0;
}

int CodeGenerator::visit(If &ifStatement)
{
    ifStatement.expression->visit(this);
    
    int label = generateLabel();
    std::cout << "    if " << res << " == r0 goto if_" << label << std::endl;

    ifStatement.block->visit(this);

    std::cout << "if_" << label << ":" << std::endl;

    return 0;
}

int CodeGenerator::visit(While &statement)
{
    int label = generateLabel();

    std::cout << "while_" << label << ":" << std::endl;
    statement.expression->visit(this);
    std::cout << "    if " << res << " == r0 goto while_end_" << label << std::endl;

    statement.block->visit(this);

    std::cout << "    jmp while_" << label << std::endl;
    std::cout << "while_end_" << label << ":" << std::endl;

    return 0;
}

int CodeGenerator::visit(For &statement)
{
    int label = generateLabel();
    statement.clause1->visit(this);

    std::cout << "for_" << label << ":" << std::endl;
    statement.expression2->visit(this);
    std::cout << "    if " << res << " == r0 goto for_end_" << label << std::endl;

    statement.block->visit(this);

    statement.expression3->visit(this);
    std::cout << "    jmp for_" << label << std::endl;
    std::cout << "for_end_" << label << ":" << std::endl;

    return 0;
}

int CodeGenerator::visit(GotoStatement &gotoStatement)
{
    std::cout << "    jmp " << gotoStatement.label << std::endl;
    return 0;
}

int CodeGenerator::visit(LabelStatement &label)
{
    IdentifierExpr *l = dynamic_cast<IdentifierExpr *>(label.label.get());
    std::cout << l->name << ":" << std::endl;
    return 0;
}

int CodeGenerator::visit(AssignmentExpr &expr)
{
    expr.lhs->visit(this);
    std::string left = res;
    expr.rhs->visit(this);
    std::string right = res;
    std::cout << "    " << left << " " << Assignment::to_str(Assignment::Kind(expr.kind))
        << " " << right << std::endl;
    return 0;
}
