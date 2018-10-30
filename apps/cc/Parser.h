#ifndef CC__PARSER__H
#define CC__PARSER__H

#include <memory>
#include <string>
#include <vector>
#include "Types.h"
#include "TranslationUnit.h"

class Expression;
typedef std::unique_ptr<Expression> ExpressionPtr;

class Struct;
typedef std::shared_ptr<Struct> StructPtr;

struct Token {
    int token;
    std::string text;

    Type type() const;
    bool isAssignment() const;
    static bool isAssignment(int token);
    bool isRelational() const;
    int to_int() const;
    static bool isBinaryOp(int token);
};

class Parser {
public:
    int parse(const char *filename);

    TranslationUnit unit;

protected:
    Token token;

    int readToken();
    int peekToken(int pos = 0);

    int expect(const std::string &str);

    VariablePtr createVariable(const DeclarationSpecifier &declSpec, const std::string &name);

    Type parseType();
    int parseDeclarationSpecifiers(DeclarationSpecifier &declSpec);
    int parseParameters(FunctionPtr &function);
    bool parseVariableDefinition(VariablePtr &var);
    ExpressionPtr parseExpression();
    int parseArguments(std::vector<ExpressionPtr> &arguments);
    StatementBlockPtr parseStatementBlock();
    int parseFunction(Type type, const std::string &name);
    StatementPtr parseIf();
    StatementPtr parseWhile();
    StatementPtr parseFor();
    StructPtr parseStruct();

    int checkDimensions(int next);

    std::vector<Token> nextTokens;
    FILE *file = nullptr;
};

#endif//CC__PARSER__H
