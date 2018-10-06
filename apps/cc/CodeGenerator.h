#ifndef CC__CODE_GENERATOR__H
#define CC__CODE_GENERATOR__H

#include <bitset>
#include "Visitor.h"

class CodeGenerator: public Visitor {
public:
    int visit(Function &f);
    int visit(StatementBlock &block);
    int visit(ReturnStatement &ret);
    int visit(FunctionCall &f);

protected:
    class Scope {
    public:
        std::string functionName;
    };

    int getFreeRegister();

    Scope scope;
    std::bitset<32> usedRegisters;
    bool isLeaf;
    bool returnSeen;
};

#endif//CC__CODE_GENERATOR__H
