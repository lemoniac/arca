#include "Expression.h"
#include "Statement.h"
#include "Visitor.h"

int ReturnStatement::visit(Visitor *visitor) { visitor->visit(*this); }
int FunctionCall::visit(Visitor *visitor) { visitor->visit(*this); }
int StatementBlock::visit(Visitor *visitor) { visitor->visit(*this); }
int Assignment::visit(Visitor *visitor) { visitor->visit(*this); }
int If::visit(Visitor *visitor) { visitor->visit(*this); }
int While::visit(Visitor *visitor) { visitor->visit(*this); }
int For::visit(Visitor *visitor) { visitor->visit(*this); }
int GotoStatement::visit(Visitor *visitor) { visitor->visit(*this); }
int LabelStatement::visit(Visitor *visitor) { visitor->visit(*this); }

void StatementBlock::add(StatementPtr statement)
{
    statement->setParent(this);
    statements.push_back(std::move(statement));
}

void Statement::setParent(StatementBlock *parent)
{
    this->parent = parent;
}

void If::setParent(StatementBlock *parent)
{
    Statement::setParent(parent);
    block->symbolTable->parent = parent->symbolTable.get();
}

void While::setParent(StatementBlock *parent)
{
    Statement::setParent(parent);
    block->symbolTable->parent = parent->symbolTable.get();
}

void For::setParent(StatementBlock *parent)
{
    Statement::setParent(parent);
    block->symbolTable->parent = parent->symbolTable.get();
}


const char *Assignment::to_str(Assignment::Kind kind)
{
    switch(kind)
    {
        case Assignment::Kind::Assign: return "=";
        case Assignment::Kind::Add: return "+=";
        case Assignment::Kind::Sub: return "-=";
        case Assignment::Kind::Mul: return "*=";
    }

    return "?";
}

const char *Assignment::to_str() const
{
    return to_str(kind);
}
