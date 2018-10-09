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

struct Token {
    int token;
    std::string text;

    Type type() const
    {
        switch(token)
        {
            case INT: return Type::Int;
            case VOID: return Type::Void;
        }

        return Type::Error;
    }

    bool isAssignment() const
    {
        switch(token)
        {
            case '=': return true;
            case ADD_ASSIGN: return true;
            case SUB_ASSIGN: return true;
            case MUL_ASSIGN: return true;
            case DIV_ASSIGN: return true;
            case MOD_ASSIGN: return true;
            case AND_ASSIGN: return true;
            case OR_ASSIGN: return true;
            case XOR_ASSIGN: return true;
            case LEFT_ASSIGN: return true;
            case RIGHT_ASSIGN: return true;
        }
        return false;
    }
};

std::vector<Token> nextTokens;

Token token;

int read_token()
{
    if(!nextTokens.empty())
    {
        token = nextTokens.back();
        nextTokens.pop_back();
        return 1;
    }

    token.token = yylex();
    if(token.token == 0)
        return 0;

    token.text = std::string(yytext);
    return 1;
}

int peek_token()
{
    int next = yylex();
    if(next > 0)
    {
        Token token = {next, yytext};
        nextTokens.push_back(token);
    }

    return next;
}

Type parse_type()
{
    read_token();
    token.type();
}

int valid_identifier(const char *id)
{
    return 0;
}

int expect(const std::string &str)
{
    read_token();
    if(str != token.text)
        return -1;
    return 0;
}

#define EXPECT(str, ret) \
    if(expect(str) < 0) \
    { \
        printf("error: line %u found %s expected %s\n", yylineno, token.text.c_str(), str); \
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
    var.type = token.type();
    ERROR(var.type == Type::Error, "Expected type")
    read_token();
    if(token.token == '*')
    {
        var.is_pointer = true;
        read_token();
    }
    var.name = token.text;
    read_token();

    function->parameters.push_back(var);

    if(token.token == ')')
        return 0;

    if(token.token == ',')
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
    var.type = token.type();
    read_token();
    var.name = token.text;
    read_token();
    if(token.token == ';')
    {
        printf("var %i %s\n", (int)var.type, var.name.c_str());
        return var;
    }
    if(token.token == '=')
    {
        read_token();
        std::string varvalue = token.text;
        EXPECT(";", var);
        printf("var %i %s %s\n", (int)var.type, var.name.c_str(), varvalue.c_str());
        var.value = std::stoi(varvalue);
        var.valueSet = true;
        return var;
    }

    printf("error: expected ';' or '='");
    return var;
}

ExpressionPtr parse_expression()
{
    ExpressionPtr res;
    read_token();
    switch(token.token)
    {
        case IDENTIFIER: {
            auto expr = std::make_unique<IdentifierExpr>();
            expr->name = token.text;
            res = std::move(expr);
            break;
        }

        case I_CONSTANT: {
            auto expr = std::make_unique<IntConstant>();
            expr->value = std::stoi(token.text);
            res = std::move(expr);
            break;
        }

        default:
            std::cerr << "error: invalid expression '" << token.text << "'" << std::endl;
            return 0;
    }

    int next = peek_token();
    if (next == ';' || next == ')' || next == 0)
        return res;

    if (next == '+')
    {
        read_token();
        auto op = std::make_unique<BinaryOpExpr>();
        op->left = std::move(res);
        op->op = BinaryOpExpr::Op::Add;
        op->right = parse_expression();

        return std::move(op);
    }

    return 0;
}

int parse_arguments(std::vector<ExpressionPtr> &arguments)
{
    read_token();
    while(token.token != ')')
    {
        arguments.push_back(parse_expression());
        read_token();
        if(token.token == ',')
            read_token();
    }
}

int parse_statement_block(FunctionPtr &function)
{
    read_token();
    // local variables
    while(token.type() != Type::Error)
    {
        function->statements.locals.push_back(parse_variable_definition());
        read_token();
    }

    // statements
    do {
        switch(token.token)
        {
            case WHILE:
            {
                EXPECT("(", -1);
                break;
            }

            case RETURN:
            {
                auto ret = std::make_unique<ReturnStatement>();
                if(peek_token() != ';')
                {
                    ret->returnValue = parse_expression();
                }
                EXPECT(";", -1);
                function->statements.statements.push_back(std::move(ret));
                break;
            }

            case IDENTIFIER: {
                std::string identifier = token.text;
                read_token();
                if(token.isAssignment())
                {
                    auto assignment = std::make_unique<Assignment>();
                    assignment->parent = &function->statements;
                    assignment->dest = identifier;
                    assignment->expression = parse_expression();
                    EXPECT(";", -1);
                    function->statements.statements.push_back(std::move(assignment));
                }
                else if(token.token == '(')
                {
                    auto call = std::make_unique<FunctionCall>();
                    call->function = identifier;
                    call->parent = &function->statements;
                    parse_arguments(call->arguments);
                    function->statements.statements.push_back(std::move(call));
                    EXPECT(";", -1);
                }
                else if(token.token == ';')
                {}
                else
                    std::cerr << yylineno << ": unexpected token " << token.text << std::endl;
                break;
            }

            default:
                std::cerr << yylineno << ": unexpected token " << token.text << std::endl;
        }
        read_token();
    } while(token.token != 0 && token.token != '}');

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
    function->name = token.text;

    EXPECT("(", -1);

    read_token();

    if(token.token != ')')
    {
        if(parse_parameters(function) < 0)
            return -1;
    }

    read_token();
    if(token.token == ';')
    {
        unit.functions.push_back(std::move(function));
        return 0;
    }
    if(token.token == '{')
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
