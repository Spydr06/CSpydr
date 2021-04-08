#include "exprParser.h"
#include "AST.h"
#include "parser.h"

#include <stdio.h>
#include <string.h>

#define SYNTAX_ERROR(msg) throwSyntaxError(parser->lexer->errorHandler, msg, parser->lexer->srcPath, parser->lexer->line, parser->lexer->iInLine)

static ASTExpr_T* parserParseIdExpr(parser_T* parser)
{
    char* id = parser->token->value;
    parserConsume(parser, TOKEN_ID, "expect variable or function call");

    switch(parser->token->type)
    {
        case TOKEN_LEFT_PAREN: {
            return initASTExpr(AST_EX_FN_CALL, parserParseFunctionCall(parser, id));
        }
        default:
            return initASTExpr(AST_EX_VAR_CALL, initASTVarCall(id));
    }

    //this should never happen
    return NULL;
}

static ASTExprClosure_T* parserParseClosure(parser_T* parser)
{
    parserConsume(parser, TOKEN_LEFT_PAREN, "expect \"(\" before closure");
    ASTExprClosure_T* ast = initASTClosure(parserParseExpr(parser));
    parserConsume(parser, TOKEN_RIGHT_PAREN, "expect \")\" after closure");
    return ast;
}

ASTExprConstant_T* parserParseNumber(parser_T* parser)
{
    int integer = atoi(parser->token->value);
    parserConsume(parser, TOKEN_NUMBER, "expect a number");
    ASTExprConstant_T* _const = initASTConstant(initASTDataType_T(AST_I32), NULL);
    _const->value = malloc(sizeof(int));
    memcpy(_const->value, &integer, sizeof(int));

    return _const;
}

static ASTExprConstant_T* parserParseString(parser_T* parser)
{
    char* _string = parser->token->value;
    parserConsume(parser, TOKEN_STR, "expect a string");

    ASTExprConstant_T* _const =  initASTConstant(initASTDataType_T(AST_STR), _string);
    _const->value = calloc(strlen(_string) + 1, sizeof(char));
    strcpy(_const->value, _string);
    
    return _const;
}

static ASTExprConstant_T* parserParseChar(parser_T* parser)
{
    char character = parser->token->value[0];
    parserConsume(parser, TOKEN_CHAR, "expect a char");

    ASTExprConstant_T* _const = initASTConstant(initASTDataType_T(AST_CHAR), &character);
    _const->value = malloc(sizeof(char));

    return _const;
}

static ASTExprConstant_T* parserParseBool(parser_T* parser)
{
    int8_t boolean = strcmp(parser->token->value, "true") == 0 ? 1 : 0;
    parserConsume(parser, TOKEN_BOOL, "expect a bool");

    ASTExprConstant_T* _const = initASTConstant(initASTDataType_T(AST_BOOL), &boolean);
    _const->value = malloc(sizeof(int8_t));

    return _const;
}

static void parserParseNil(parser_T* parser)
{
    parserConsume(parser, TOKEN_NIL, "expect nil");
}

static ASTExprNot_T* parserParseNot(parser_T* parser)
{
    parserConsume(parser, TOKEN_BANG, "expect \"!\" for bool not operator");
    return initASTNot(parserParseExpr(parser));
}

static ASTExprNegate_T* parserParseNegate(parser_T* parser)
{
    parserConsume(parser, TOKEN_MINUS, "expect \"-\" for negation operator");
    return initASTNegate(parserParseExpr(parser));
}

static ASTExprOp_T* parserParseAddition(parser_T* parser, ASTExpr_T* left, bool removeOpSymbol)
{
    if(removeOpSymbol)
    {
        parserConsume(parser, TOKEN_PLUS, "expect \"+\" for addition operator");
    }

    return initASTOp(AST_OP_ADD, left, parserParseExpr(parser));
}

static ASTExprOp_T* parserParseSubtraction(parser_T* parser, ASTExpr_T* left, bool removeOpSymbol)
{
    if(removeOpSymbol)
    {
        parserConsume(parser, TOKEN_MINUS, "expect \"-\" for subtraction operator");
    }

    return initASTOp(AST_OP_SUB, left, parserParseExpr(parser));
}

static ASTExprOp_T* parserParseMultiplication(parser_T* parser, ASTExpr_T* left, bool removeOpSymbol)
{
    if(removeOpSymbol)
    {
        parserConsume(parser, TOKEN_STAR, "expect \"*\" for multiplication operator");
    }

    return initASTOp(AST_OP_MULT, left, parserParseExpr(parser));
}

static ASTExprOp_T* parserParseDivision(parser_T* parser, ASTExpr_T* left, bool removeOpSymbol)
{
    if(removeOpSymbol)
    {
        parserConsume(parser, TOKEN_SLASH, "expect \"/\" for division operator");
    }

    return initASTOp(AST_OP_DIV, left, parserParseExpr(parser));
}

static ASTExprArray_T* parserParseArrayExpr(parser_T* parser)
{
    ASTExprArray_T* ast = initASTArrayExpr();

    parserConsume(parser, TOKEN_LEFT_BRACKET, "expect \"[\" for array expression");
    while(parser->token->type != TOKEN_RIGHT_BRACKET)
    {
        listPush(ast->indices, parserParseExpr(parser));
        if(parser->token->type != TOKEN_RIGHT_BRACKET)
        {
            parserConsume(parser, TOKEN_COMMA, "expect \",\" between or \"]\" after array indices");
        }
    }
    parserAdvance(parser);

    return ast;
}

static ASTExprBoolOp_T* parserParseAnd(parser_T* parser, ASTExpr_T* left)
{
    parserConsume(parser, TOKEN_AND_AND, "expect \"&&\" for and operator");

    return initASTBoolOp(AST_OP_AND, left, parserParseExpr(parser));
}

static ASTExprBoolOp_T* parserParseOr(parser_T* parser, ASTExpr_T* left)
{
    parserConsume(parser, TOKEN_OR_OR, "expect \"||\" for or operator");

    return initASTBoolOp(AST_OP_OR, left, parserParseExpr(parser));
}

static ASTExprBoolOp_T* parserParseEquals(parser_T* parser, ASTExpr_T* left)
{
    parserConsume(parser, TOKEN_EQUALS_EQUALS, "expect \"==\" for equals operator");

    return initASTBoolOp(AST_OP_EQ, left, parserParseExpr(parser));
}

static ASTExprBoolOp_T* parserParseNotEquals(parser_T* parser, ASTExpr_T* left)
{
    parserConsume(parser, TOKEN_BANG_EQUALS, "expect \"!=\" for not equals operator");

    return initASTBoolOp(AST_OP_NE, left, parserParseExpr(parser));
}

static ASTExprBoolOp_T* parserParseGreater(parser_T* parser, ASTExpr_T* left)
{
    parserConsume(parser, TOKEN_GREATER, "expect \">\" for greater operator");

    return initASTBoolOp(AST_OP_GT, left, parserParseExpr(parser));
}

static ASTExprBoolOp_T* parserParseLess(parser_T* parser, ASTExpr_T* left)
{
    parserConsume(parser, TOKEN_LESS, "expect \"<\" for less operator");

    return initASTBoolOp(AST_OP_LT, left, parserParseExpr(parser));
}

static ASTExprBoolOp_T* parserParseGreaterEquals(parser_T* parser, ASTExpr_T* left)
{
    parserConsume(parser, TOKEN_GREATER_EQUALS, "expect \">=\" for greater equals operator");

    return initASTBoolOp(AST_OP_GE, left, parserParseExpr(parser));
}

static ASTExprBoolOp_T* parserParseLessEquals(parser_T* parser, ASTExpr_T* left)
{
    parserConsume(parser, TOKEN_LESS_EQUALS, "expect \"<=\" for less equals operator");

    return initASTBoolOp(AST_OP_LE, left, parserParseExpr(parser));
}

ASTExpr_T* parserParseExpr(parser_T* parser)
{
    void* ptr = NULL;
    ASTExprType_T type;

    switch(parser->token->type)
    {
        case TOKEN_NUMBER:
            ptr = parserParseNumber(parser);
            type = AST_EX_CONSTANT;
            break;
        case TOKEN_BOOL:
            ptr = parserParseBool(parser);
            type = AST_EX_CONSTANT;
            break;
        case TOKEN_STR:
            ptr =  parserParseString(parser);
            type = AST_EX_CONSTANT;
            break;
        case TOKEN_CHAR:
            ptr =  parserParseChar(parser);
            type = AST_EX_CONSTANT;
            break;
        case TOKEN_NIL:
            parserParseNil(parser);
            type = AST_EX_NIL;
            break;

        case TOKEN_BANG:
            ptr =  parserParseNot(parser);
            type = AST_EX_NOT;
            break;
        case TOKEN_MINUS:
            ptr =  parserParseNegate(parser);
            type = AST_EX_NEGATE;
            break;

        case TOKEN_LEFT_PAREN:
            ptr =  parserParseClosure(parser);
            type = AST_EX_CLOSURE;
            break;

        case TOKEN_ID: {
            ASTExpr_T* id = parserParseIdExpr(parser);
            type = id->type;
            ptr = id->expr;
        } break;

        case TOKEN_LEFT_BRACKET:
            ptr =  parserParseArrayExpr(parser);
            type = AST_EX_ARRAY;
            break;

        default: {
            const char* template = "unexpected token \"%s\"; expect expression";
            char* errorMsg = calloc(strlen(template) + strlen(parser->token->value) + 1, sizeof(char));

            sprintf(errorMsg, template, parser->token->value);
            SYNTAX_ERROR(errorMsg);
            exit(1);
        }
    }

    ASTExpr_T* expr = initASTExpr(type, ptr);
    switch(parser->token->type)
    {
        case TOKEN_PLUS:
            return initASTExpr(AST_EX_OP, parserParseAddition(parser, expr, true));
        case TOKEN_MINUS:
            return initASTExpr(AST_EX_OP, parserParseSubtraction(parser, expr, true));
        case TOKEN_STAR:
            return initASTExpr(AST_EX_OP, parserParseMultiplication(parser, expr, true));
        case TOKEN_SLASH:
            return initASTExpr(AST_EX_OP, parserParseDivision(parser, expr, true));

        case TOKEN_OR_OR:
            return initASTExpr(AST_EX_BOOL_OP, parserParseOr(parser, expr));
        case TOKEN_AND_AND:
            return initASTExpr(AST_EX_BOOL_OP, parserParseAnd(parser, expr));
        case TOKEN_GREATER:
            return initASTExpr(AST_EX_BOOL_OP, parserParseGreater(parser, expr));
        case TOKEN_LESS:
            return initASTExpr(AST_EX_BOOL_OP, parserParseLess(parser, expr));
        case TOKEN_EQUALS_EQUALS:
            return initASTExpr(AST_EX_BOOL_OP, parserParseEquals(parser, expr));
        case TOKEN_GREATER_EQUALS:
            return initASTExpr(AST_EX_BOOL_OP, parserParseGreaterEquals(parser, expr));
        case TOKEN_LESS_EQUALS:
            return initASTExpr(AST_EX_BOOL_OP, parserParseLessEquals(parser, expr));
        case TOKEN_BANG_EQUALS:
            return initASTExpr(AST_EX_BOOL_OP, parserParseNotEquals(parser, expr));

        default:
            break;
    }
    return expr;

    //this should never happen
    return NULL;
}

ASTAssignment_T* parserParseAssinment(parser_T* parser, char* targetCallee)
{
    ASTExprVarCall_T* left = initASTVarCall(targetCallee);
    ASTExpr_T* right = NULL;

    switch(parser->token->type)
    {
        case TOKEN_EQUALS:
            parserAdvance(parser);
            right = parserParseExpr(parser);
            break;

        case TOKEN_PLUS_EQUALS:
            parserAdvance(parser);
            right = initASTExpr(AST_EX_OP, parserParseAddition(parser, initASTExpr(AST_EX_VAR_CALL, left), false));
            break;

        case TOKEN_MINUS_EQUALS:
            parserAdvance(parser);
            right = initASTExpr(AST_EX_OP, parserParseSubtraction(parser, initASTExpr(AST_EX_VAR_CALL, left), false));
            break;

        case TOKEN_STAR_EQUALS:
            parserAdvance(parser);
            right = initASTExpr(AST_EX_OP, parserParseMultiplication(parser, initASTExpr(AST_EX_VAR_CALL, left), false));
            break;

        case TOKEN_SLASH_EQUALS:
            parserAdvance(parser);
            right = initASTExpr(AST_EX_OP, parserParseDivision(parser, initASTExpr(AST_EX_VAR_CALL, left), false));
            break;

        default: {
            const char* template = "unexpected token \"%s\"; expect assignment";
            char* errorMsg = calloc(strlen(template) + strlen(parser->token->value) + 1, sizeof(char));

            sprintf(errorMsg, template, parser->token->value);
            SYNTAX_ERROR(errorMsg);
            exit(1);
        }
    }

    return initASTAssingment(left, right);
}


ASTExprFnCall_T* parserParseFunctionCall(parser_T* parser, char* callee)
{
    parserAdvance(parser);
    ASTExprFnCall_T* ast = initASTFunctionCall(callee);

    while(parser->token->type != TOKEN_RIGHT_PAREN)
    {
        listPush(ast->args, parserParseExpr(parser));
        if(parser->token->type != TOKEN_RIGHT_PAREN)
        {
            parserConsume(parser, TOKEN_COMMA, "expect \",\" between or \")\" after arguments");
        }
    }
    parserConsume(parser, TOKEN_RIGHT_PAREN, "expect \")\" after function call");

    return ast;
}


ASTLocal_T* parserParseLocal(parser_T* parser, bool includeLet)
{
    if(includeLet) {
        parserConsume(parser, TOKEN_LET, "expect local variable definition keyword \"let\"");
    }
    char* name = parser->token->value;
    parserConsume(parser, TOKEN_ID, "expect variable name");
    parserConsume(parser, TOKEN_COLON, "expect \":\" after variable name");

    ASTDataType_T* dataType = parserParseDataType(parser);
    ASTExpr_T* expr = initASTExprNil();

    if(parser->token->type == TOKEN_EQUALS)
    {
        parserAdvance(parser);
        free(expr);
        expr = parserParseExpr(parser);
    }

    return initASTLocal(name, dataType, expr);
}   