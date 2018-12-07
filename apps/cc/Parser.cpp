#include <iostream>
#include "Parser.h"
#include "lex.yy.c"
#include "Function.h"
#include "Expression.h"
#include "Statement.h"
#include "Struct.h"
#include "Variable.h"

void yyerror(const char *error)
{
    std::cerr << error << std::endl;
}

Type Token::type() const
{
    return type(token);
}

Type Token::type(int token)
{
    switch(token)
    {
        case INT:
        case LONG:
        case UNSIGNED: return Type::Int;
        case SHORT: return Type::Short;
        case VOID: return Type::Void;
        case CHAR: return Type::Char;
        case STRUCT: return Type::Struct;
    }

    return Type::Error;
}

bool Token::isAssignment() const
{
    return isAssignment(token);
}

bool Token::isAssignment(int token)
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
    if(text.size() > 2 && text[0] == '0' && text[1] == 'x')
    {
        if(text.size() > 10)
        {
            std::cerr << yylineno << ": warning: number too big " << text << std::endl;
            return -1;
        }
        return std::stoi(std::string(text.begin() + 2, text.end()), nullptr, 16);
    }
    else if(text.size() == 3 && text[0] == '\'' && text[2] == '\'')
        return text[1];
    else
        return std::stoi(text);
}

std::string Token::to_str() const
{
    if(text.size() <= 2)
        return text;

    return std::string(text.begin() + 1, text.end() - 1);
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
        case AND_OP:
        case OR_OP:
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
        case '&': return BinaryOpExpr::Op::And;
        case '|': return BinaryOpExpr::Op::Or;
        case '^': return BinaryOpExpr::Op::Xor;
        case LEFT_OP: return BinaryOpExpr::Op::LShift;
        case RIGHT_OP: return BinaryOpExpr::Op::RShift;
        case EQ_OP: return BinaryOpExpr::Op::Eq;
        case NE_OP: return BinaryOpExpr::Op::NEq;
        case '<': return BinaryOpExpr::Op::LT;
        case '>': return BinaryOpExpr::Op::GT;
        case LE_OP: return BinaryOpExpr::Op::LE;
        case GE_OP: return BinaryOpExpr::Op::GE;
        case AND_OP: return BinaryOpExpr::Op::And;
        case OR_OP: return BinaryOpExpr::Op::Or;
    }

    throw std::runtime_error("unknown binary operator");
}

AssignmentExpr::Kind token_to_assignment(int token)
{
    switch(token)
    {
        case '=': return AssignmentExpr::Kind::Assign;
        case ADD_ASSIGN: return AssignmentExpr::Kind::Add;
        case SUB_ASSIGN: return AssignmentExpr::Kind::Sub;
        case MUL_ASSIGN: return AssignmentExpr::Kind::Mul;
    }

    throw std::runtime_error("unknown assignment operator");
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
        fprintf(stderr, "error: line %i found %s expected %s\n", yylineno, token.text.c_str(), str); \
        return ret; \
    }

#define ERROR(a, error_msg) \
    if((a)) \
    { \
        fprintf(stderr, "%s\n", error_msg); \
        return -1; \
    }

#define NOT_NULL(expr, ret) if ((expr) == nullptr) return ret;

int Parser::parseDeclarationSpecifiers(DeclarationSpecifier &declSpec)
{
    declSpec = DeclarationSpecifier();

    int next = peekToken();
    if(next == STATIC)
    {
        readToken();
        declSpec.isStatic = true;
        next = peekToken();
    }
    if(next == CONST)
    {
        readToken();
        declSpec.isConst = true;
        next = peekToken();
    }

    declSpec.type = Token::type(next);
    if(declSpec.type == Type::Error)
        return -1;

    readToken();
    if(declSpec.type == Type::Struct)
    {
        readToken();
        declSpec.structName = token.text;
    }

    next = peekToken();
    if(next == '*')
    {
        readToken();
        declSpec.isPointer = true;
    }

    return 0;
}


int Parser::parseParameters(FunctionPtr &function)
{
    auto var = std::make_unique<Variable>();
    parseDeclarationSpecifiers(var->declSpec);
    readToken();
    var->name = token.text;
    readToken();

    function->parameters.push_back(std::move(var));

    if(token.token == ')')
        return 0;

    if(token.token == ',')
        return parseParameters(function);

    printf("error: expected ',' or ')'");
    return -1;
}

VariablePtr Parser::createVariable(const DeclarationSpecifier &declSpec, const std::string &name)
{
    auto var = std::make_unique<Variable>();
    var->declSpec = declSpec;
    var->name = name;
    return var;
}

bool Parser::parseVariableDefinition(VariablePtr &var)
{
    readToken();
    var->value = parseExpression();
    var->valueSet = true;
    return true;
}

ExpressionPtr Parser::parseExpression()
{
    ExpressionPtr res;
    readToken();
    switch(token.token)
    {
        case IDENTIFIER:
            res = std::make_unique<IdentifierExpr>(token.text);
            break;

        case I_CONSTANT:
            res = std::make_unique<IntConstant>(token.to_int());
            break;

        case STRING_LITERAL:
            res = std::make_unique<StringLiteral>(token.text);
            break;

        case '(': {
            auto par = std::make_unique<ParentExpr>();
            NOT_NULL(par->expr = parseExpression(), 0);
            res = std::move(par);
            EXPECT(")", 0);
            break;
        }

        case SIZEOF:
        {
            EXPECT("(", 0);
            auto s = std::make_unique<UnaryOpExpr>(UnaryOpExpr::Op::SizeOf);
            NOT_NULL(s->expr = parseExpression(), 0);
            res = std::move(s);
            EXPECT(")", 0);
            break;
        }

        case INC_OP:
        case DEC_OP:
        case '-':
        case '&':
        case '*':
        {
            auto s = std::make_unique<UnaryOpExpr>(UnaryOpExpr::from_token(token.token));
            NOT_NULL(s->expr = parseExpression(), 0);
            res = std::move(s);
            break;
        }

        case '{':
            res = parseInitializerList();
            break;

        default:
            std::cerr << "error: invalid expression '" << token.text << "'" << std::endl;
            return 0;
    }

    int next = peekToken();
    if (next == ';' || next == ')' || next == ',' || next == ']' || next == ':' || next == 0)
        return res;

    while(next == '[')
    {
        readToken();
        auto subscript = std::make_unique<SubscriptExpr>();
        subscript->lhs = std::move(res);
        NOT_NULL(subscript->rhs = parseExpression(), 0);
        EXPECT("]", 0);
        res = std::move(subscript);
        next = peekToken();
    }

    if(next == '.' || next == PTR_OP)
    {
        readToken();
        readToken();
        auto member = std::make_unique<MemberExpr>();
        member->name = token.text;
        member->parent = std::move(res);
        res = std::move(member);
        next = peekToken();
    }

    if (Token::isBinaryOp(next))
    {
        readToken();
        auto op = std::make_unique<BinaryOpExpr>(token_to_op(next));
        op->left = std::move(res);
        NOT_NULL(op->right = parseExpression(), 0);

        return std::move(op);
    }

    if(Token::isAssignment(next))
    {
        auto expr = std::make_unique<AssignmentExpr>();
        expr->lhs = std::move(res);
        readToken();
        expr->kind = token_to_assignment(token.token);
        NOT_NULL(expr->rhs = parseExpression(), 0);
        return expr;
    }

    if(next == '(')
    {
        readToken();
        auto call = std::make_unique<FunctionCallExpr>();
        call->function = std::move(res);
        if(parseArguments(call->arguments) < 0) return 0;
        return call;
    }

    return res;
}

int Parser::parseArguments(std::vector<ExpressionPtr> &arguments)
{
    int next = peekToken();
    if(next == ')')
    {
        readToken();
        return 0;
    }

    while(next != ')')
    {
        auto expr = parseExpression();
        NOT_NULL(expr, -1);
        arguments.push_back(std::move(expr));
        readToken();
        if(token.token == ',')
            next = peekToken();
        else if(token.token == ')')
            break;
        else
        {
            std::cerr << "error: unexpected token" << std::endl;
            return -1;
        }
    }

    return 0;
}

StatementBlockPtr Parser::parseStatementBlock()
{
    auto block = std::make_unique<StatementBlock>();

    // local variables
    DeclarationSpecifier declSpec;
    while(parseDeclarationSpecifiers(declSpec) >= 0)
    {
        readToken();
        auto var = createVariable(declSpec, token.text);
        int next = peekToken();
        int dim = checkDimensions(next);

        if(next == '=' && !parseVariableDefinition(var))
            return 0;
        var->elems = dim;
        EXPECT(";", 0);
        block->locals.push_back(std::move(var));
    }

    int next = peekToken();

    StatementBlock *oldBlock = currentBlock;
    currentBlock = block.get();

    // statements
    while(next != 0 && next != '}') {
        auto statement = parseStatement();
        NOT_NULL(statement, 0);
        block->add(std::move(statement));
        next = peekToken();
    };

    EXPECT("}", 0);

    currentBlock = oldBlock;

    return block;
}

StatementPtr Parser::parseStatement()
{
    StatementPtr statement;
    int next = peekToken();

    switch(next)
    {
        case WHILE: readToken(); statement = parseWhile(); break;
        case IF: readToken(); statement = parseIf(); break;
        case FOR: readToken(); statement = parseFor(); break;

        case RETURN:
        {
            readToken();
            auto ret = std::make_unique<ReturnStatement>();
            if(peekToken() != ';')
                NOT_NULL(ret->returnValue = parseExpression(), 0);
            EXPECT(";", 0);

            statement = std::move(ret);
            break;
        }

        case GOTO: {
            readToken();
            readToken();
            statement = std::make_unique<GotoStatement>(token.text);
            EXPECT(";", 0);
            break;
        }

        case ASM: {
            readToken();
            EXPECT("(", 0);
            readToken();
            auto a = std::make_unique<AsmStatement>();
            a->statement = token.to_str();
            EXPECT(")", 0);
            EXPECT(";", 0);
            statement = std::move(a);
            break;
        }

        case '{':
        {
            readToken();
            auto inblock = parseStatementBlock();
            inblock->setParent(currentBlock);
            statement = std::move(inblock);
            break;
        }

        default:
        {
            auto expr = parseExpression();
            next = peekToken();
            if(next == ':')
            {
                auto l = std::make_unique<LabelStatement>();
                l->label = std::move(expr);
                statement = std::move(l);
            }
            else
            {
                EXPECT(";", 0);
                statement = std::move(expr);
            }
        }
    }

    return statement;
}

int Parser::parseFunction(Type type, const std::string &name)
{
    auto function = std::make_unique<Function>();
    function->returnType = type;
    function->name = name;

    EXPECT("(", -1);

    int next = peekToken();
    if(next == ')')
        readToken();
    else
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
    s->statement = parseStatement();

    return s;
}

StatementPtr Parser::parseWhile()
{
    auto s = std::make_unique<While>();
    EXPECT("(", 0);
    s->expression = parseExpression();
    EXPECT(")", 0);
    s->statement = parseStatement();

    return s;
}

StatementPtr Parser::parseFor()
{
    auto f = std::make_unique<For>();
    EXPECT("(", 0);
    f->clause1 = parseExpression();
    EXPECT(";", 0);
    f->expression2 = parseExpression();
    EXPECT(";", 0);
    f->expression3 = parseExpression();
    EXPECT(")", 0);
    //EXPECT("{", 0);
    f->statement = parseStatement();

    return f;
}

StructPtr Parser::parseStruct()
{
    auto s = std::make_unique<Struct>();

    readToken();
    s->name = token.text;
    int next = peekToken();
    if(next == '{')
    {
        EXPECT("{", 0);
        next = peekToken();
        while(next != '}')
        {
            DeclarationSpecifier declSpec;
            if(parseDeclarationSpecifiers(declSpec) < 0) return 0;
            readToken();
            std::string identifier = token.text;
            next = peekToken();
            declSpec.elems = checkDimensions(next);
            EXPECT(";", 0);
            s->member.push_back(Struct::Member{declSpec, identifier});
            next = peekToken();
        }
        EXPECT("}", 0);
    }
    else if(next == IDENTIFIER)
    {
        readToken();
        s->varname = token.text;
    }
    else if(next == ';')
    {
        // forward
    }
    else
    {
        readToken();
        std::cout << yylineno << ": unexpected token: " << token.text << std::endl;
        return 0;
    }
    EXPECT(";", 0);

    return s;
}

ExpressionPtr Parser::parseInitializerList()
{
    auto list = std::make_unique<InitializerListExpr>();

    int next = peekToken();
    while(next != '}')
    {
        auto expr = parseExpression();
        list->elements.push_back(std::move(expr));
        next = peekToken();
        if(next == ',')
            readToken();
    }

    EXPECT("}", 0);

    return list;
}


int Parser::parse(const char *filename)
{
    file = fopen(filename, "rt");
    yyin = file;

    readToken();
    while(token.token > 0)
    {
        if(token.token == STRUCT || token.token == UNION)
        {
            auto s = parseStruct();
            if(!s)
                return -1;

            if(s->varname.empty())
            {
                unit.symbolTable.add(s->name, Type::Struct, nullptr, s);
            }
            else
            {
                DeclarationSpecifier declSpec;
                declSpec.type = Type::Struct;
                declSpec.structName = s->name;

                auto var = createVariable(declSpec, s->varname);
                var->isGlobal = true;
                unit.globals.push_back(std::move(var));
            }
        }
        else
        {
            DeclarationSpecifier declSpec;

            if(token.token == STATIC) { declSpec.isStatic = true; readToken(); }
            if(token.token == CONST) { declSpec.isConst = true; readToken(); }

            declSpec.type = token.type();
            if(declSpec.type == Type::Error)
            {
                std::cerr << yylineno << ": unexpected token: " << token.text << std::endl;
                return -1;
            }
            int next = peekToken();
            if(next == '*')
            {
                declSpec.isPointer = true;
                readToken();
            }
            readToken();
            std::string name = token.text;
            next = peekToken();
            int dim = checkDimensions(next);

            if(next == '(')
            {
                if(parseFunction(declSpec.type, name) < 0)
                    return -1;
            }
            else if(next == '=' || next == ';')
            {
                auto var = createVariable(declSpec, name);
                if(next == '=' && !parseVariableDefinition(var))
                    return -1;
                EXPECT(";", -1);
                var->isGlobal = true;
                var->elems = dim;
                unit.globals.push_back(std::move(var));
            }
        }
        readToken();
    }

    return 0;
}

int Parser::checkDimensions(int &next)
{
    int res = 0;
    if(next == -1)
        next = peekToken();
    while(next == '[')
    {
        readToken();
        readToken();
        res = token.to_int();
        EXPECT("]", 0);
        next = peekToken();
    }

    return res;
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

int Parser::peekToken(int pos)
{
    if(pos >= nextTokens.size())
    {
        int next = yylex();
        if(next > 0)
        {
            Token token = {next, yytext};
            nextTokens.push_back(token);
        }

        return next;
    }

    return nextTokens.back().token;
}
