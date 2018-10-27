#include <stdio.h>
#include <string>
#include <iostream>
#include <vector>
#include <memory>

#include "Parser.h"

#include "Expression.h"
#include "Function.h"
#include "Statement.h"
#include "Variable.h"
#include "Visitor.h"
#include "CodeGenerator.h"
#include "SymbolTable.h"
#include "SymbolTablePass.h"
#include "TranslationUnit.h"
#include "PrintVisitor.h"
#include "Preprocessor.h"
#include "SimplifyExpressions.h"

int main(int argc, char **argv)
{
    //Preprocessor cpp;
    //cpp.preprocess(argv[1]);

    Parser parser;
    parser.parse(argv[1]);

    SymbolTablePass stp(parser.unit);
    if(stp.visit(parser.unit) < 0)
        return 1;

    SimplifyExpressions simplifier;
    simplifier.visit(parser.unit);
/*
    PrintVisitor v;
    v.visit(parser.unit);
*/

    CodeGenerator cg;
    cg.visit(parser.unit);

    return 0;
}
