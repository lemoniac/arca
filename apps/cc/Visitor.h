#ifndef CC__VISITOR__H
#define CC__VISITOR__H

class Function;
class StatementBlock;
class If;
class While;
class For;
class ReturnStatement;
class FunctionCall;
class TranslationUnit;
class Assignment;
class GotoStatement;
class LabelStatement;
class AsmStatement;

class IntConstant;
class StringLiteral;
class IdentifierExpr;
class MemberExpr;
class SubscriptExpr;
class ParentExpr;
class BinaryOpExpr;
class UnaryOpExpr;
class AssignmentExpr;
class FunctionCallExpr;

#define VISITOR_VISIT(T)     virtual int visit(T &) { return 0; }

class Visitor {
public:
    virtual int visit(TranslationUnit &unit) { return 0; }
    virtual int visit(Function &function) { return 0; }
    virtual int visit(StatementBlock &block) { return 0; }
    virtual int visit(If &ifStatement) { return 0; }
    virtual int visit(While &ifStatement) { return 0; }
    virtual int visit(For &forStatement) { return 0; }
    virtual int visit(ReturnStatement &ret) { return 0; }
    virtual int visit(FunctionCall &function) { return 0; }
    virtual int visit(Assignment &assignment) { return 0; }
    virtual int visit(GotoStatement &gotoStatement) { return 0; }
    virtual int visit(LabelStatement &label) { return 0; }
    VISITOR_VISIT(AsmStatement);

    virtual int visit(IntConstant &constant) { return 0; }
    virtual int visit(StringLiteral &constant) { return 0; }
    virtual int visit(IdentifierExpr &identifier) { return 0; }
    virtual int visit(MemberExpr &identifier) { return 0; }
    VISITOR_VISIT(SubscriptExpr)
    virtual int visit(ParentExpr &par) { return 0; }
    virtual int visit(BinaryOpExpr &op) { return 0; }
    virtual int visit(UnaryOpExpr &op) { return 0; }
    virtual int visit(AssignmentExpr &expr) { return 0; }
    virtual int visit(FunctionCallExpr &function) { return 0; }
};

#endif//CC__VISITOR__H
