#ifndef CC__CODE_GENERATOR__H
#define CC__CODE_GENERATOR__H

#include <bitset>
#include <vector>
#include "Visitor.h"

class SymbolTable;

class CodeGenerator: public Visitor {
public:
    int visit(Function &f);
    int visit(StatementBlock &block);
    int visit(ReturnStatement &ret);
    int visit(FunctionCall &f);
    int visit(TranslationUnit &unit);
    int visit(Assignment &assignment);

    int visit(IntConstant &constant);
    int visit(IdentifierExpr &identifier);
    int visit(BinaryOpExpr &op);

protected:
    class Scope {
    public:
        std::string functionName;
        SymbolTable *symbols = nullptr;
    };

    int getFreeRegister();

    std::vector<Scope> scope;
    std::bitset<32> usedRegisters;
    bool isLeaf;
    bool returnSeen;
    int rdest = -1;
    std::string res;
};

#endif//CC__CODE_GENERATOR__H
