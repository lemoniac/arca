#ifndef CC__VISITOR__H
#define CC__VISITOR__H

class Function;
class StatementBlock;
class If;
class While;
class ReturnStatement;
class FunctionCall;
class TranslationUnit;
class Assignment;
class GotoStatement;
class LabelStatement;

class IntConstant;
class IdentifierExpr;
class ParentExpr;
class BinaryOpExpr;

class Visitor {
public:
    virtual int visit(TranslationUnit &unit) { return 0; }
    virtual int visit(Function &function) { return 0; }
    virtual int visit(StatementBlock &block) { return 0; }
    virtual int visit(If &ifStatement) { return 0; }
    virtual int visit(While &ifStatement) { return 0; }
    virtual int visit(ReturnStatement &ret) { return 0; }
    virtual int visit(FunctionCall &function) { return 0; }
    virtual int visit(Assignment &assignment) { return 0; }
    virtual int visit(GotoStatement &gotoStatement) { return 0; }
    virtual int visit(LabelStatement &label) { return 0; }

    virtual int visit(IntConstant &constant) { return 0; }
    virtual int visit(IdentifierExpr &identifier) { return 0; }
    virtual int visit(ParentExpr &par) { return 0; }
    virtual int visit(BinaryOpExpr &op) { return 0; }
};


#endif//CC__VISITOR__H
