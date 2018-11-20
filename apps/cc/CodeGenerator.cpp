#include <iostream>
#include "CodeGenerator.h"
#include "Function.h"
#include "TranslationUnit.h"
#include "Expression.h"
#include "Struct.h"

int size(Type type)
{
    switch(type)
    {
        case Type::Char: return 1;
        case Type::Short: return 2;
        case Type::Int: return 4;
        default:
            return -1;

    }
}

const char *type_to_str(const DeclarationSpecifier &declSpec)
{
    if(declSpec.isPointer)
        return "int";

    switch(declSpec.type)
    {
        case Type::Char: return "char";
        case Type::Int: return "int";
        case Type::Short: return "uint16";
        default: return "???";
    }
}

std::string Res::to_string() const
{
    std::string res;
    if(type == Res::Reg)
    {
        if(pointer)
            res = "*";
        else if(ref)
            res = "&";
        res += "r" + std::to_string(reg);

        if(offset > 0)
            res += " + " + std::to_string(offset);

        if(!member.empty())
            res += "." + name + "." + member;
    }
    else if(type == Res::Imm)
        res = std::to_string(imm);
    else
    {
        if(ref)
            res = "&";
        res += name;
    }

    return res;
}

Res Res::R(int r)
{
    Res res;
    res.type = Res::Reg;
    res.reg = r;
    return res;
}

Res Res::I(int v)
{
    Res res;
    res.type = Res::Imm;
    res.imm = v;
    return res;
}

Res Res::Sym(const std::string &name)
{
    Res res;
    res.type = Res::Symbol;
    res.ref = true;
    res.name = name;
    return res;
}

Res Res::Ptr(int r)
{
    Res res;
    res.type = Res::Reg;
    res.pointer = true;
    res.reg = r;
    return res;
}


int CodeGenerator::visit(Function &f)
{
    if(!f.statements)
        return 0;

    functionName = f.name;
    std::cout << f.name << ":" << std::endl;

    isLeaf = true;
    returnSeen = false;
    usedRegisters = 0;
    usedRegisters[14] = usedRegisters[15] = 1; // SP, LR
    unsigned soff = 0;
    for(unsigned i = 0; i < f.parameters.size(); i++)
    {
        int r = i + 1;
        f.parameters[i]->reg = r;
        usedRegisters[r] = 1;

        auto &p = f.parameters[i];
        if(p->referenced)
        {
            //auto s = scope.back().symbols->find(p->name);
            std::cout << "    r14 = r14 - 4" /*<< s->size()*/ << std::endl;
            std::cout << "    *r14 = r" << r << std::endl;
            p->stackOffset = soff;
            soff+= 4;
        }
    }

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
            std::cout << "    r" << r << " = " << res.to_string() << std::endl;
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
        std::cout << "    r1 = " << res.to_string() << std::endl;
    }

    if(isLeaf)
        std::cout << "    ret" << std::endl;
    else
        std::cout << "    jmp " << functionName << "_epilogue" << std::endl;

    return 0;
}

int CodeGenerator::visit(TranslationUnit &unit)
{
    this->unit = &unit;

    Scope s = {&unit.symbolTable};
    scope.push_back(s);

    bool initializeGlobals = !unit.strings.empty();
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
        if(s->type == Type::Struct && s->variable == nullptr)
        {
            std::cout << ".struct " << s->name << std::endl;
            for(const auto &m : s->structInfo->member)
                std::cout << "    " << type_to_str(m.declSpec) << " " << m.name << std::endl;
            std::cout << ".endstruct" << std::endl;
            std::cout << std::endl;
        }
    }

    for(const auto &s : unit.strings)
        std::cout << "    char " << s.second << " = " << s.first << std::endl;

    for(auto &g : unit.globals)
    {
        if(g->declSpec.type == Type::Struct)
        {
            std::cout << "    struct " << g->declSpec.structName << " " << g->name << std::endl;
        }
        else
        {
            std::cout << "    " << type_to_str(g->declSpec) << " " << g->name;
            if(g->elems > 0)
                std::cout << "[" << g->elems << "]";
            else {
                if(g->valueSet && g->isConstant())
                    std::cout << " = " << g->getValue();
                else
                    std::cout << " = 0";
            }
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

int CodeGenerator::allocateRegister()
{
    int r = getFreeRegister();
    usedRegisters[r] = true;
    return r;
}


int CodeGenerator::generateLabel()
{
    labelCounter++;
    return labelCounter;
}

int CodeGenerator::visit(IntConstant &constant)
{
    res = Res::I(constant.value);
    return 0;
}

int CodeGenerator::visit(StringLiteral &str)
{
    res = Res();
    res.ref = true;
    res.name = unit->strings[str.value];
    return 0;
}

int CodeGenerator::visit(IdentifierExpr &identifier)
{
    if(!identifier.symbol)
        throw std::runtime_error("undefined identifier: " + identifier.name);

    auto &s = identifier.symbol;

    if(s->variable->isGlobal)
    {
        int r = allocateRegister();
        int r2 = allocateRegister();
        s->variable->reg = r;
        std::cout << "    r" << r << " = &" << s->name << std::endl;
        std::cout << "    r" << r2 << " = *r" << r << std::endl;
        res = Res::R(r2);
    }
    else if(s->type == Type::Int || s->type == Type::Char ||
        (s->variable && s->variable->declSpec.isPointer) || s->type == Type::Struct)
    {
        res = Res::R(s->variable->reg);
    }

    return 0;
}

int CodeGenerator::visit(MemberExpr &member)
{
    member.parent->visit(this);

    IdentifierExpr *identifier = dynamic_cast<IdentifierExpr *>(member.parent.get());
    const auto symbol = scope.back().symbols->find(identifier->name);

    res.name = symbol->variable->declSpec.structName;
    res.member = member.name;
    res.pointer = true;

    return 0;
}

int CodeGenerator::visit(SubscriptExpr &expr)
{
    int r = getFreeRegister();
    usedRegisters[r] = 1;

    expr.lhs->visit(this);
    auto lhs = res;
    expr.rhs->visit(this);
    auto rhs = res;

    std::cout << "    r" << r << " = " << lhs.to_string() << " + " << rhs.to_string() << std::endl;

    int r2 = getFreeRegister();
    usedRegisters[r2] = 1;
    const char *as = "=";
    if(expr.lhs->type() == Type::Char)
        as = "b=";
    std::cout << "    r" << r2 << " " << as << " *r" << r << std::endl;
    res = Res::R(r2);
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

    std::cout << "    r" << r << " = " << left.to_string() << " " << op.to_str() << " " << right.to_string() << std::endl;
    res = Res::R(r);
    return 0;
}

int CodeGenerator::visit(UnaryOpExpr &op)
{
    switch(op.op)
    {
        case UnaryOpExpr::Op::PreInc:
            op.expr->visit(this);
            std::cout << "    " << res.to_string() << " = " << res.to_string() << " + 1" << std::endl;
            break;

        case UnaryOpExpr::Op::PreDec:
            op.expr->visit(this);
            std::cout << "    " << res.to_string() << " = " << res.to_string() << " - 1" << std::endl;
            break;

        case UnaryOpExpr::Op::Neg: {
            op.expr->visit(this);
            int r = getFreeRegister();
            usedRegisters[r] = 1;
            std::cout << "    r" << r << " = r0 - " << res.to_string() << std::endl;
            res = Res::R(r);
            break;
        }

        case UnaryOpExpr::Op::AddrOf: {
            const IdentifierExpr *id = dynamic_cast<const IdentifierExpr *>(op.expr.get());
            if(id)
            {
                if(id->symbol->variable->isGlobal)
                    res = Res::Sym(id->name);
                else
                {
                    res = Res::R(14);
                    res.offset = id->symbol->variable->stackOffset;
                }
            }
            break;
        }

        case UnaryOpExpr::Op::Ref: {
            const IdentifierExpr *id = dynamic_cast<const IdentifierExpr *>(op.expr.get());
            if(id)
            {
                if(id->symbol->variable->isGlobal)
                    res = Res::Sym(id->name);
                else
                    res = Res::Ptr(id->symbol->variable->reg);
            }
            break;
        }
    }

    return 0;
}

int CodeGenerator::visit(If &ifStatement)
{
    ifStatement.expression->visit(this);
    
    int label = generateLabel();
    std::cout << "    if " << res.to_string() << " == r0 goto if_" << label << std::endl;

    ifStatement.statement->visit(this);

    std::cout << "if_" << label << ":" << std::endl;

    return 0;
}

int CodeGenerator::visit(While &statement)
{
    int label = generateLabel();

    std::cout << "while_" << label << ":" << std::endl;
    statement.expression->visit(this);
    std::cout << "    if " << res.to_string() << " == r0 goto while_end_" << label << std::endl;

    statement.statement->visit(this);

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
    std::cout << "    if " << res.to_string() << " == r0 goto for_end_" << label << std::endl;

    statement.statement->visit(this);

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

int CodeGenerator::visit(AsmStatement &statement)
{
    std::cout << "    " << statement.statement << std::endl;
    return 0;
}


int CodeGenerator::visit(AssignmentExpr &expr)
{
    std::bitset<32> oldRegisters = usedRegisters;

    expr.lhs->visit(this);
    Res left = res;
    expr.rhs->visit(this);
    Res right = res;
    if(left.pointer && right.type == Res::Imm)
    {
        int r = allocateRegister();
        res = Res::R(r);
        std::cout << "    " << res.to_string() << " = " << right.to_string() << std::endl;
        right = res;
    }
    else if(left.type == Res::Symbol)
    {
        if(right.type == Res::Imm)
        {
            int r = allocateRegister();
            res = Res::R(r);
            std::cout << "    " << res.to_string() << " = " << right.to_string() << std::endl;
            right = res;
        }

        int r = allocateRegister();
        res = Res::R(r);
        std::cout << "    " << res.to_string() << " = " << left.to_string() << std::endl;
        res.pointer = true;
        left = res;
    }

    std::cout << "    " << left.to_string() << " " << AssignmentExpr::to_str(AssignmentExpr::Kind(expr.kind))
        << " " << right.to_string() << std::endl;

    usedRegisters = oldRegisters;

    return 0;
}

int CodeGenerator::visit(FunctionCallExpr &f)
{
    int r = 1;
    for(const auto &arg : f.arguments)
    {
        arg->visit(this);
        std::cout << "    r" << r << " = " << res.to_string() << std::endl;
        r++;
    }
    IdentifierExpr *function = dynamic_cast<IdentifierExpr *>(f.function.get());
    std::cout << "    call " << function->name << std::endl;
    res = Res::R(1);
}
