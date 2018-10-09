#include "Expression.h"
#include "Visitor.h"

int IntConstant::visit(Visitor *visitor) { visitor->visit(*this); }
int IdentifierExpr::visit(Visitor *visitor) { visitor->visit(*this); }
int BinaryOpExpr::visit(Visitor *visitor) { visitor->visit(*this); }
