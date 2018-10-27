#include <iostream>
#include "Expression.h"
#include "Statement.h"
#include "Visitor.h"

int ReturnStatement::visit(Visitor *visitor) { visitor->visit(*this); }
int FunctionCall::visit(Visitor *visitor) { visitor->visit(*this); }
int StatementBlock::visit(Visitor *visitor) { visitor->visit(*this); }
int If::visit(Visitor *visitor) { visitor->visit(*this); }
int While::visit(Visitor *visitor) { visitor->visit(*this); }
int For::visit(Visitor *visitor) { visitor->visit(*this); }
int GotoStatement::visit(Visitor *visitor) { visitor->visit(*this); }
int LabelStatement::visit(Visitor *visitor) { visitor->visit(*this); }

void StatementBlock::add(StatementPtr statement)
{
    if(!statement)
    {
        std::cerr << "error: trying to add an empty statement" << std::endl;
        return;
    }

    statement->setParent(this);
    statements.push_back(std::move(statement));
}

void Statement::setParent(StatementBlock *parent)
{
    this->parent = parent;
}

void StatementBlock::setParent(StatementBlock *parent)
{
    Statement::setParent(parent);
    symbolTable->parent = parent->symbolTable.get();
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
