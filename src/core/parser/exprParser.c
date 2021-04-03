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

        case TOKEN_PLUS: {
            
        } break;

        case TOKEN_MINUS: {

        } break;

        case TOKEN_STAR: {

        } break;

        case TOKEN_SLASH: {

        } break;

        default:
            return initASTExpr(AST_EX_VAR_CALL, initASTVarCall(id));
    }

    //this should never happen
    return NULL;
}

static ASTExpr_T* parserParseClosure(parser_T* parser)
{
    parserConsume(parser, TOKEN_LEFT_PAREN, "expect \"(\" before closure");
    ASTExprClosure_T* ast = initASTClosure(parserParseExpr(parser));
    parserConsume(parser, TOKEN_RIGHT_PAREN, "expect \")\" after closure");
    return initASTExpr(AST_EX_CLOSURE, ast);
}

static ASTExpr_T* parserParseNumber(parser_T* parser)
{
    int32_t integer = atoi(parser->token->value);
    parserConsume(parser, TOKEN_NUMBER, "expect a number");

    return initASTExpr(AST_EX_CONSTANT, initASTConstant(initASTDataType_T(AST_I32), &integer));
}

static ASTExpr_T* parserParseString(parser_T* parser)
{
    char* string = parser->token->value;
    parserConsume(parser, TOKEN_STR, "expect a string");

    return initASTExpr(AST_EX_CONSTANT, initASTConstant(initASTDataType_T(AST_STR), string));
}

static ASTExpr_T* parserParseChar(parser_T* parser)
{
    char character = parser->token->value[0];
    parserConsume(parser, TOKEN_CHAR, "expect a char");

    return initASTExpr(AST_EX_CONSTANT, initASTConstant(initASTDataType_T(AST_CHAR), &character));
}

static ASTExpr_T* parserParseBool(parser_T* parser)
{
    int8_t boolean = strcmp(parser->token->value, "true") == 0 ? 1 : 0;
    parserConsume(parser, TOKEN_BOOL, "expect a bool");

    return initASTExpr(AST_EX_CONSTANT, initASTConstant(initASTDataType_T(AST_BOOL), &boolean));
}

static ASTExpr_T* parserParseNil(parser_T* parser)
{
    parserConsume(parser, TOKEN_NIL, "expect nil");
    return initASTExpr(AST_EX_NIL, initASTExprNil());
}

static ASTExpr_T* parserParseNot(parser_T* parser)
{
    parserConsume(parser, TOKEN_BANG, "expect \"!\" for bool not operator");
    return initASTExpr(AST_EX_NOT, initASTNot(parserParseExpr(parser)));
}

static ASTExpr_T* parserParseNegate(parser_T* parser)
{
    parserConsume(parser, TOKEN_MINUS, "expect \"-\" for negation operator");
    return initASTExpr(AST_EX_NEGATE, initASTNot(parserParseExpr(parser)));
}

static ASTExpr_T* parserParseAddition(parser_T* parser, ASTExpr_T* left, bool removeOpSymbol)
{
    if(removeOpSymbol)
    {
        parserConsume(parser, TOKEN_PLUS, "expect \"+\" for addition operator");
    }

    return initASTExpr(AST_EX_OP, initASTOp(AST_OP_ADD, left, parserParseExpr(parser)));
}

static ASTExpr_T* parserParseSubtraction(parser_T* parser, ASTExpr_T* left, bool removeOpSymbol)
{
    if(removeOpSymbol)
    {
        parserConsume(parser, TOKEN_PLUS, "expect \"-\" for subtraction operator");
    }

    return initASTExpr(AST_EX_OP, initASTOp(AST_OP_SUB, left, parserParseExpr(parser)));
}

static ASTExpr_T* parserParseMultiplication(parser_T* parser, ASTExpr_T* left, bool removeOpSymbol)
{
    if(removeOpSymbol)
    {
        parserConsume(parser, TOKEN_PLUS, "expect \"*\" for multiplication operator");
    }

    return initASTExpr(AST_EX_OP, initASTOp(AST_OP_MULT, left, parserParseExpr(parser)));
}

static ASTExpr_T* parserParseDivision(parser_T* parser, ASTExpr_T* left, bool removeOpSymbol)
{
    if(removeOpSymbol)
    {
        parserConsume(parser, TOKEN_PLUS, "expect \"/\" for division operator");
    }

    return initASTExpr(AST_EX_OP, initASTOp(AST_OP_DIV, left, parserParseExpr(parser)));
}

ASTExpr_T* parserParseExpr(parser_T* parser)
{
    switch(parser->token->type)
    {
        case TOKEN_NUMBER:
            return parserParseNumber(parser);
        case TOKEN_BOOL:
            return parserParseBool(parser);
        case TOKEN_STR:
            return parserParseString(parser);
        case TOKEN_CHAR:
            return parserParseChar(parser);
        case TOKEN_NIL:
            return parserParseNil(parser);

        case TOKEN_BANG:
            return parserParseNot(parser);
        case TOKEN_MINUS:
            return parserParseNegate(parser);

        case TOKEN_LEFT_PAREN:
            return parserParseClosure(parser);

        case TOKEN_ID:
            return parserParseIdExpr(parser);

        default: {
            const char* template = "unexpected token \"%s\"; expect expression";
            char* errorMsg = calloc(strlen(template) + strlen(parser->token->value) + 1, sizeof(char));

            sprintf(errorMsg, template, parser->token->value);
            SYNTAX_ERROR(errorMsg);
            exit(1);
        }
    }

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
            right = parserParseAddition(parser, initASTExpr(AST_EX_VAR_CALL, left), false);
            break;

        case TOKEN_MINUS_EQUALS:
            parserAdvance(parser);
            right = parserParseSubtraction(parser, initASTExpr(AST_EX_VAR_CALL, left), false);
            break;

        case TOKEN_STAR_EQUALS:
            parserAdvance(parser);
            right = parserParseMultiplication(parser, initASTExpr(AST_EX_VAR_CALL, left), false);
            break;

        case TOKEN_SLASH_EQUALS:
            parserAdvance(parser);
            right = parserParseDivision(parser, initASTExpr(AST_EX_VAR_CALL, left), false);
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