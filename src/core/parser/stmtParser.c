#include "stmtParser.h"
#include "AST.h"
#include "exprParser.h"

#include <stdbool.h>
#include <string.h>

ASTIfStmt_T* parserParseIf(parser_T* parser)
{
    parserConsume(parser, TOKEN_STMT, "expect if statement");
    ASTIfStmt_T* ast = initASTIfStmt(parserParseExpr(parser));
    ast->ifBody = parserParseCompound(parser);

    if(parser->token->type == TOKEN_STMT && strcmp(parser->token->value, "else") == 0)
    {
        parserConsume(parser, TOKEN_STMT, "expect else statement");
        ast->elseBody = parserParseCompound(parser);
    }
    return ast;
}

ASTForStmt_T* parserParseFor(parser_T* parser)
{
    parserConsume(parser, TOKEN_STMT, "expect for statement");

    ASTLocal_T* counterVar = NULL;
    if(parser->token->type != TOKEN_UNDERSCORE)
    {
        counterVar = parserParseLocal(parser, false);
    }
    else {
        parserAdvance(parser);
    }
    parserConsume(parser, TOKEN_SEMICOLON, "expect \";\" after first for argument");
    ASTExpr_T* condition = parserParseExpr(parser);
    parserConsume(parser, TOKEN_SEMICOLON, "expect \";\" after second for agrument");

    ASTAssignment_T* increment = NULL;
    if(parser->token->type != TOKEN_UNDERSCORE)
    {
        char* callee = parser->token->value;
        parserConsume(parser, TOKEN_ID, "expect variable name for assignment");
        increment = parserParseAssinment(parser, callee);
    }
    else {
        parserAdvance(parser);
    }

    ASTForStmt_T* ast = initASTForStmt(counterVar, condition, increment);
    ast->body = parserParseCompound(parser);

    return ast;
}

ASTWhileStmt_T* parserParseWhile(parser_T* parser)
{
    parserConsume(parser, TOKEN_STMT, "expect while statement");
    ASTWhileStmt_T* ast = initASTWhileStmt(parserParseExpr(parser));
    ast->body = parserParseCompound(parser);

    return ast;
}

ASTReturnStmt_T* parserParseReturnStmt(parser_T* parser)
{
    parserConsume(parser, TOKEN_STMT, "expect return statement");
    return initASTReturnStmt(parserParseExpr(parser));
}

ASTExitStmt_T* parserParseExitStmt(parser_T* parser)
{
    parserConsume(parser, TOKEN_STMT, "expect exit statement");
    return initASTExitStmt(parserParseExpr(parser));
}