#include "Statement.h"
#include "Visitor.h"

int ReturnStatement::visit(Visitor *visitor) { visitor->visit(*this); }
int FunctionCall::visit(Visitor *visitor) { visitor->visit(*this); }
int StatementBlock::visit(Visitor *visitor) { visitor->visit(*this); }
