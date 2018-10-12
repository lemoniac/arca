#include "Statement.h"
#include "Visitor.h"

int ReturnStatement::visit(Visitor *visitor) { visitor->visit(*this); }
int FunctionCall::visit(Visitor *visitor) { visitor->visit(*this); }
int StatementBlock::visit(Visitor *visitor) { visitor->visit(*this); }
int Assignment::visit(Visitor *visitor) { visitor->visit(*this); }
int If::visit(Visitor *visitor) { visitor->visit(*this); }
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
    block->symbolTable->parent = parent->symbolTable;
}
