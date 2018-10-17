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

bool Token::isRelational() const
{
    switch(token)
    {
        case EQ_OP: return true;
        case NE_OP: return true;
        case '<': return true;
        case '>': return true;
        case LE_OP: return true;
        case GE_OP: return true;
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
        case EQ_OP:
        case NE_OP:
        case '<':
        case '>':
        case LE_OP:
        case GE_OP:
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
        case EQ_OP: return BinaryOpExpr::Op::Eq;
        case NE_OP: return BinaryOpExpr::Op::NEq;
        case '<': return BinaryOpExpr::Op::LT;
        case '>': return BinaryOpExpr::Op::GT;
        case LE_OP: return BinaryOpExpr::Op::LE;
        case GE_OP: return BinaryOpExpr::Op::GE;
    }

    throw std::runtime_error("unknown binary operator");
}


Type Parser::parseType()
{
    readToken();
    return token.type();
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

VariablePtr Parser::parseVariableDefinition(Type type, const std::string &name)
{
    auto var = std::make_unique<Variable>();
    var->type = type;
    var->name = name;
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

        case '(': {
            res = parseExpression();
            EXPECT(")", 0);
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
        Type type = token.type();
        readToken();
        block->locals.push_back(parseVariableDefinition(type, token.text));
        readToken();
    }

    // statements
    do {
        switch(token.token)
        {
            case WHILE: block->add(parseWhile()); break;
            case IF: block->add(parseIf()); break;

            case RETURN:
            {
                auto ret = std::make_unique<ReturnStatement>();
                if(peekToken() != ';')
                {
                    ret->returnValue = parseExpression();
                }
                EXPECT(";", 0);
                block->add(std::move(ret));
                break;
            }

            case IDENTIFIER: {
                std::string identifier = token.text;
                readToken();
                if(token.isAssignment())
                {
                    auto assignment = std::make_unique<Assignment>();
                    assignment->dest = identifier;
                    assignment->expression = parseExpression();
                    EXPECT(";", 0);
                    block->add(std::move(assignment));
                }
                else if(token.token == '(')
                {
                    auto call = std::make_unique<FunctionCall>();
                    call->function = identifier;
                    parseArguments(call->arguments);
                    block->add(std::move(call));
                    EXPECT(";", 0);
                }
                else if(token.token == ':')
                {
                    auto l = std::make_unique<LabelStatement>();
                    l->label = identifier;
                    block->add(std::move(l));
                }
                else if(token.token == ';')
                {}
                else
                    std::cerr << yylineno << ": unexpected token " << token.text << std::endl;
                break;
            }

            case GOTO: {
                readToken();
                auto g = std::make_unique<GotoStatement>();
                g->label = token.text;
                block->add(std::move(g));
                EXPECT(";", 0);
                break;
            }

            default:
                std::cerr << yylineno << ": unexpected token " << token.text << std::endl;
        }
        readToken();
    } while(token.token != 0 && token.token != '}');

    return block;
}

int Parser::parseFunction(Type type, const std::string &name)
{
    auto function = std::make_unique<Function>();
    function->returnType = type;
    function->name = name;

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

StatementPtr Parser::parseWhile()
{
    auto s = std::make_unique<While>();
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

    readToken();
    while(token.token > 0)
    {
        Type type = token.type();
        if(type == Type::Error)
            return -1;
        readToken();
        std::string name = token.text;
        int next = peekToken();
        if(next == '(')
            parseFunction(type, name);
        else if(next == '=' || next == ';')
        {
            auto var = parseVariableDefinition(type, name);
            var->isGlobal = true;
            unit.globals.push_back(std::move(var));
        }
        readToken();
    }

    return 0;
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
