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
    int visit(If &ifStatement);
    int visit(While &statement);
    int visit(For &statement);
    int visit(FunctionCall &f);
    int visit(TranslationUnit &unit);
    int visit(Assignment &assignment);
    int visit(GotoStatement &gotoStatement);
    int visit(LabelStatement &label);

    int visit(IntConstant &constant);
    int visit(IdentifierExpr &identifier);
    int visit(ParentExpr &expr);
    int visit(BinaryOpExpr &op);

protected:
    class Scope {
    public:
        SymbolTable *symbols = nullptr;
    };

    int getFreeRegister();

    int generateLabel();

    std::vector<Scope> scope;
    std::bitset<32> usedRegisters;
    bool isLeaf;
    bool returnSeen;
    std::string functionName;
    int rdest = -1;
    std::string res;

    int labelCounter = 0;
};

#endif//CC__CODE_GENERATOR__H
