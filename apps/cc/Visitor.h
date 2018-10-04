#ifndef CC__VISITOR__H
#define CC__VISITOR__H

class Function;
class StatementBlock;
class ReturnStatement;

class Visitor {
public:
    virtual int visit(Function &function) = 0;
    virtual int visit(StatementBlock &block) = 0;
    virtual int visit(ReturnStatement &ret) = 0;
};


#endif//CC__VISITOR__H
