#include <iostream>
#include "Parser.h"
#include "lex.yy.c"
#include "Function.h"
#include "Expression.h"
#include "Statement.h"
#include "Variable.h"

void yyerror(const char *error)
{
    std::cerr << error << std::endl;
}

Type Token::type() const
{
    switch(token)
    {
        case INT: return Type::Int;
        case VOID: return Type::Void;
    }

    return Type::Error;
}

bool Token::isAssignment() const
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

int Token::to_int() const
{
    return std::stoi(text);
}

bool Token::isBinaryOp(int token)
{
    switch(token)
    {
        case '+':
        case '-':
        case '*':
        case '/':
        case '%':
        case '|':
        case '&':
        case '^':
        case LEFT_OP:
        case RIGHT_OP:
            return true;
    }

    return false;
}

BinaryOpExpr::Op token_to_op(int token)
{
    switch(token)
    {
        case '+': return BinaryOpExpr::Op::Add;
        case '-': return BinaryOpExpr::Op::Sub;
        case '*': return BinaryOpExpr::Op::Mul;
        case '/': return BinaryOpExpr::Op::Div;
        case '%': return BinaryOpExpr::Op::Mod;
        case '|': return BinaryOpExpr::Op::And;
        case '&': return BinaryOpExpr::Op::Or;
        case '^': return BinaryOpExpr::Op::Xor;
        case LEFT_OP: return BinaryOpExpr::Op::LShift;
        case RIGHT_OP: return BinaryOpExpr::Op::RShift;
    }

    throw std::runtime_error("unknown binary operator");
}


Type Parser::parseType()
{
    readToken();
    token.type();
}

int Parser::expect(const std::string &str)
{
    readToken();
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


int Parser::parseParameters(FunctionPtr &function)
{
    auto var = std::make_unique<Variable>();
    var->type = token.type();
    ERROR(var->type == Type::Error, "Expected type")
    readToken();
    if(token.token == '*')
    {
        var->is_pointer = true;
        readToken();
    }
    var->name = token.text;
    readToken();

    function->parameters.push_back(std::move(var));

    if(token.token == ')')
        return 0;

    if(token.token == ',')
    {
        readToken();
        return parseParameters(function);
    }

    printf("error: expected ',' or ')'");
    return -1;
}

VariablePtr Parser::parseVariableDefinition()
{
    auto var = std::make_unique<Variable>();
    var->type = token.type();
    readToken();
    var->name = token.text;
    readToken();
    if(token.token == ';')
    {
        printf("var %i %s\n", (int)var->type, var->name.c_str());
        return var;
    }
    if(token.token == '=')
    {
        readToken();
        std::string varvalue = token.text;
        EXPECT(";", var);
        printf("var %i %s %s\n", (int)var->type, var->name.c_str(), varvalue.c_str());
        var->value = std::stoi(varvalue);
        var->valueSet = true;
        return var;
    }

    printf("error: expected ';' or '='");
    return var;
}

ExpressionPtr Parser::parseExpression()
{
    ExpressionPtr res;
    readToken();
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
            expr->value = token.to_int();
            res = std::move(expr);
            break;
        }

        default:
            std::cerr << "error: invalid expression '" << token.text << "'" << std::endl;
            return 0;
    }

    int next = peekToken();
    if (next == ';' || next == ')' || next == 0)
        return res;

    if (Token::isBinaryOp(next))
    {
        readToken();
        auto op = std::make_unique<BinaryOpExpr>();
        op->left = std::move(res);
        op->op = token_to_op(next);
        op->right = parseExpression();

        return std::move(op);
    }

    return 0;
}

int Parser::parseArguments(std::vector<ExpressionPtr> &arguments)
{
    readToken();
    while(token.token != ')')
    {
        arguments.push_back(parseExpression());
        readToken();
        if(token.token == ',')
            readToken();
    }
}

StatementBlockPtr Parser::parseStatementBlock()
{
    auto block = std::make_unique<StatementBlock>();
    readToken();
    // local variables
    while(token.type() != Type::Error)
    {
        block->locals.push_back(parseVariableDefinition());
        readToken();
    }

    // statements
    do {
        switch(token.token)
        {
            case WHILE:
            {
                EXPECT("(", 0);
                break;
            }

            case IF: block->statements.push_back(parseIf()); break;

            case RETURN:
            {
                auto ret = std::make_unique<ReturnStatement>();
                if(peekToken() != ';')
                {
                    ret->returnValue = parseExpression();
                }
                EXPECT(";", 0);
                block->statements.push_back(std::move(ret));
                break;
            }

            case IDENTIFIER: {
                std::string identifier = token.text;
                readToken();
                if(token.isAssignment())
                {
                    auto assignment = std::make_unique<Assignment>();
                    assignment->parent = block.get();
                    assignment->dest = identifier;
                    assignment->expression = parseExpression();
                    EXPECT(";", 0);
                    block->statements.push_back(std::move(assignment));
                }
                else if(token.token == '(')
                {
                    auto call = std::make_unique<FunctionCall>();
                    call->function = identifier;
                    call->parent = block.get();
                    parseArguments(call->arguments);
                    block->statements.push_back(std::move(call));
                    EXPECT(";", 0);
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
        readToken();
    } while(token.token != 0 && token.token != '}');

    return block;
}

int Parser::parseFunction()
{
    auto function = std::make_unique<Function>();
    function->returnType = parseType();

    if(function->returnType == Type::Error)
        return -1;
    readToken();
    function->name = token.text;

    EXPECT("(", -1);

    readToken();

    if(token.token != ')')
    {
        if(parseParameters(function) < 0)
            return -1;
    }

    readToken();
    if(token.token == ';')
    {
        unit.functions.push_back(std::move(function));
        return 0;
    }
    if(token.token == '{')
    {
        auto block = parseStatementBlock();
        if(!block)
            return -1;
        function->statements = std::move(block);
        function->statements->symbolTable->parent = &unit.symbolTable;

        unit.functions.push_back(std::move(function));
        return 0;
    }
    return -1;
}

StatementPtr Parser::parseIf()
{
    auto s = std::make_unique<If>();
    EXPECT("(", 0);
    s->expression = parseExpression();
    EXPECT(")", 0);
    EXPECT("{", 0);
    s->block = parseStatementBlock();
    //EXPECT("}", 0);

    return s;
}


int Parser::parse(const char *filename)
{
    file = fopen(filename, "rt");
    yyin = file;

    while(parseFunction() >= 0);
}

int Parser::readToken()
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

int Parser::peekToken()
{
    int next = yylex();
    if(next > 0)
    {
        Token token = {next, yytext};
        nextTokens.push_back(token);
    }

    return next;
}
