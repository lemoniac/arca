#include <stdio.h>
#include <string>
#include <iostream>
#include <vector>
#include <memory>

#include "lex.yy.c"

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


void yyerror(const char *error)
{
    std::cerr << error << std::endl;
}

FILE *file;
int token;
std::string token_text;

int read_token()
{
    token = yylex();
    if(token == 0)
        return 0;

    token_text = std::string(yytext);
    return 1;
}

Type token2type(int type)
{
    switch(type)
    {
        case INT: return Type::Int;
        case VOID: return Type::Void;
    }

    return Type::Error;
}

Type parse_type()
{
    read_token();
    return token2type(token);
}

int valid_identifier(const char *id)
{
    return 0;
}

int expect(const std::string &str)
{
    read_token();
    if(str != token_text)
        return -1;
    return 0;
}

#define EXPECT(str, ret) \
    if(expect(str) < 0) \
    { \
        printf("error: line %u found %s expected %s\n", yylineno, token_text.c_str(), str); \
        return ret; \
    }

#define ERROR(a, error_msg) \
    if((a)) \
    { \
        printf("%s\n", error_msg); \
        return -1; \
    }


int parse_parameters(FunctionPtr &function)
{
    Variable var = {};
    var.type = token2type(token);
    ERROR(var.type == Type::Error, "Expected type")
    read_token();
    if(token == '*')
    {
        var.is_pointer = true;
        read_token();
    }
    var.name = token_text;
    read_token();

    function->parameters.push_back(var);

    if(token == ')')
        return 0;

    if(token == ',')
    {
        read_token();
        return parse_parameters(function);
    }

    printf("error: expected ',' or ')'");
    return -1;
}

Variable parse_variable_definition()
{
    Variable var;
    var.type = token2type(token);
    read_token();
    var.name = token_text;
    read_token();
    if(token == ';')
    {
        printf("var %i %s\n", (int)var.type, var.name.c_str());
        return var;
    }
    if(token == '=')
    {
        read_token();
        std::string varvalue = token_text;
        EXPECT(";", var);
        printf("var %i %s %s\n", (int)var.type, var.name.c_str(), varvalue.c_str());
        var.value = std::stoi(varvalue);
        return var;
    }

    printf("error: expected ';' or '='");
    return var;
}

ExpressionPtr parse_expression()
{
    read_token();
    switch(token)
    {
        case IDENTIFIER: {
            auto expr = std::make_unique<IdentifierExpr>();
            expr->name = token_text;
            return std::move(expr);
        }

        case I_CONSTANT: {
            auto expr = std::make_unique<IntConstant>();
            expr->value = std::stoi(token_text);
            return std::move(expr);
        }
    }

    return 0;
}

int parse_arguments(std::vector<ExpressionPtr> &arguments)
{
    read_token();
    while(token != ')')
    {
        arguments.push_back(parse_expression());
        read_token();
        if(token == ',')
            read_token();
    }
}

int parse_statement_block(FunctionPtr &function)
{
    read_token();
    // local variables
    while(token2type(token) != Type::Error)
    {
        function->statements.locals.push_back(parse_variable_definition());
        read_token();
    }

    // statements
    do {
        switch(token)
        {
            case WHILE:
            {
                EXPECT("(", -1);
                break;
            }

            case RETURN:
            {
                read_token();
                auto ret = std::make_unique<ReturnStatement>();
                if(token != ';')
                    ret->returnValue = parse_expression();
                function->statements.statements.push_back(std::move(ret));
                break;
            }

            case IDENTIFIER: {
                std::string identifier = token_text;
                read_token();
                if(token == '=')
                    parse_expression();
                else if(token == '(')
                {
                    auto call = std::make_unique<FunctionCall>();
                    call->function = identifier;
                    call->parent = &function->statements;
                    parse_arguments(call->arguments);
                    function->statements.statements.push_back(std::move(call));
                    EXPECT(";", -1);
                }
                else if(token == ';')
                {}
                else
                    std::cerr << yylineno << ": unexpected token " << token_text << std::endl;
                break;
            }

            default:
                std::cerr << yylineno << ": unexpected token " << token_text << std::endl;
        }
        read_token();
    } while(token != 0 && token != '}');

    return 0;
}

int parse_function(TranslationUnit &unit)
{
    auto function = std::make_unique<Function>();
    function->returnType = parse_type();
    function->statements.symbolTable->parent = &unit.symbolTable;
    if(function->returnType == Type::Error)
        return -1;
    read_token();
    function->name = token_text;

    EXPECT("(", -1);

    read_token();

    if(token != ')')
    {
        if(parse_parameters(function) < 0)
            return -1;
    }

    read_token();
    if(token == ';')
    {
        unit.functions.push_back(std::move(function));
        return 0;
    }
    if(token == '{')
    {
        if(parse_statement_block(function) < 0)
            return -1;
        unit.functions.push_back(std::move(function));
        return 0;
    }
    return -1;
}


int main(int argc, char **argv)
{
    file = fopen(argv[1], "rt");
    yyin = file;

    TranslationUnit unit;

    while(parse_function(unit) >= 0);

    SymbolTablePass stp(unit);
    stp.visit(unit);

    PrintVisitor v;
    v.visit(unit);

    CodeGenerator cg;
    cg.visit(unit);

    return 0;
}
