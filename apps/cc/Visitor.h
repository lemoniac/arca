#ifndef CC__VISITOR__H
#define CC__VISITOR__H

class Function;
class StatementBlock;
class ReturnStatement;
class FunctionCall;
class TranslationUnit;
class Assignment;

class Visitor {
public:
    virtual int visit(TranslationUnit &unit) = 0;
    virtual int visit(Function &function) = 0;
    virtual int visit(StatementBlock &block) = 0;
    virtual int visit(ReturnStatement &ret) = 0;
    virtual int visit(FunctionCall &function) = 0;
    virtual int visit(Assignment &assignment) = 0;
};


#endif//CC__VISITOR__H
