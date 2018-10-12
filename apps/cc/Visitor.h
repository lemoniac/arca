#ifndef CC__VISITOR__H
#define CC__VISITOR__H

class Function;
class StatementBlock;
class If;
class ReturnStatement;
class FunctionCall;
class TranslationUnit;
class Assignment;
class GotoStatement;
class LabelStatement;

class IntConstant;
class IdentifierExpr;
class BinaryOpExpr;

class Visitor {
public:
    virtual int visit(TranslationUnit &unit) = 0;
    virtual int visit(Function &function) = 0;
    virtual int visit(StatementBlock &block) = 0;
    virtual int visit(If &ifStatement) = 0;
    virtual int visit(ReturnStatement &ret) = 0;
    virtual int visit(FunctionCall &function) = 0;
    virtual int visit(Assignment &assignment) = 0;
    virtual int visit(GotoStatement &gotoStatement) = 0;
    virtual int visit(LabelStatement &label) = 0;

    virtual int visit(IntConstant &constant) = 0;
    virtual int visit(IdentifierExpr &identifier) = 0;
    virtual int visit(BinaryOpExpr &op) = 0;
};


#endif//CC__VISITOR__H
