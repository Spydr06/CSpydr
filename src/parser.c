#include "parser.h"
#include "AST.h"
#include "lexer.h"
#include "list.h"
#include "token.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

parser_T* initParser(lexer_T* lexer)
{
    parser_T* parser = calloc(1, sizeof(struct PARSER_STRUCT));
    parser->lexer = lexer;
    parser->token = lexerNextToken(lexer);

    return parser;
}

token_T* parserEat(parser_T* parser, int type)
{
    if(parser->token->type != type)
    {
        printf("[Parser]: Unexpected token: '%s', was expecting '%s'\n", tokenToString(parser->token), tokenTypeToString(type));
        exit(1);
    }

    parser->token = lexerNextToken(parser->lexer);
    return parser->token;
}

void parserAdvance(parser_T* parser)
{
    parser->token = lexerNextToken(parser->lexer);
}

AST_T* parserParseNumber(parser_T* parser)
{
    int intValue = atoi(parser->token->value);
    parserEat(parser, TOKEN_NUMBER);

    AST_T* ast = initAST(AST_INT);
    ast->intValue = intValue;

    return ast;
}

AST_T* parserParseString(parser_T* parser)
{
    char* strValue = parser->token->value;
    parserEat(parser, TOKEN_STRING);

    AST_T* ast = initAST(AST_STRING);
    ast->strValue = strValue;

    return ast;
}

AST_T* parserParseList(parser_T* parser)
{
    AST_T* list = initAST(AST_COMPOUND);
    parserEat(parser, TOKEN_LEFT_PAREN);

    while(parser->token->type != TOKEN_EOF && parser->token->type != TOKEN_RIGHT_PAREN)
    {
        listPush(list->children, parserParseExpr(parser));
        if(parser->token->type != TOKEN_COMMA) {
            break;
        }
        parserEat(parser, TOKEN_COMMA);
    }

    parserEat(parser, TOKEN_RIGHT_PAREN);

    return list;
}

AST_T* parserParseId(parser_T* parser)
{
    char* value = calloc(strlen(parser->token->value) + 1, sizeof(char));
    strcpy(value, parser->token->value);

    parserEat(parser, TOKEN_ID);

    if(parser->token->type == TOKEN_EQUAL)
    {
        parserEat(parser, TOKEN_EQUAL);
        AST_T* ast = initAST(AST_ASSIGNMENT);
        ast->name = value;
        printf("Parsing assignment for '%s'...\n", ast->name);
        ast->value = parserParseExpr(parser);
        return ast;
    }

    if(parser->token->type == TOKEN_LEFT_PAREN)
    {
        AST_T* ast = initAST(AST_CALL);
        ast->name = value;
        printf("Parsing function call '%s'...\n", ast->name);
        ast->value = parserParseList(parser);
        return ast;
    }

    AST_T* ast = initAST(AST_NOOP);
    return ast;
}

AST_T* parserParseExpr(parser_T* parser)
{
    switch(parser->token->type)
    {
        case TOKEN_ID:
            return parserParseId(parser);
        case TOKEN_STRING:
            return parserParseString(parser);
        case TOKEN_NUMBER:
            return parserParseNumber(parser);
        case TOKEN_EQUAL:

        default:
            fprintf(stderr, "[ERROR] Parser: Unexpected token '%s'\n", tokenToString(parser->token));
            exit(1);
    }
}

AST_T* parserParseStmt(parser_T* parser)
{
    AST_T* stmt = initAST(AST_STATEMENT);
    stmt->name = parser->token->value;
    printf("Parsing statement '%s'...\n", stmt->name);
    parserAdvance(parser);

    stmt->value = parserParseExpr(parser);

    return stmt;
}


AST_T* parserParseCompound(parser_T* parser)
{
    AST_T* cmp = initAST(AST_COMPOUND);

    parserEat(parser, TOKEN_LEFT_BRACE);

    while(parser->token->type != TOKEN_RIGHT_BRACE && parser->token->type != TOKEN_EOF)
    {
        if(parser->token->type == TOKEN_STMT) {
            listPush(cmp->children, parserParseStmt(parser));
            parserEat(parser, TOKEN_SEMICOLON);
        }
        else if(parser->token->type == TOKEN_ID) {
            listPush(cmp->children, parserParseExpr(parser));
            parserEat(parser, TOKEN_SEMICOLON);
        }
        else if(parser->token->type == TOKEN_LET) {
            listPush(cmp->children, parserParseVarDeclaration(parser));
            parserEat(parser, TOKEN_SEMICOLON);
        }
    }

    parserEat(parser, TOKEN_RIGHT_BRACE);

    return cmp;
}

AST_T* parserParseVarDeclaration(parser_T* parser)
{
    if(parser->token->type == TOKEN_LET)
    {
        parserAdvance(parser);
    }

    AST_T* var = initAST(AST_VARIABLE);
    var->name = parser->token->value;
    parserAdvance(parser);
    printf("Parsing variable '%s'...\n", var->name);

    parserEat(parser, TOKEN_COLON);

    if(parser->token->type == TOKEN_VEC)
    {
        var->dataType = parser->token->value;

        parserAdvance(parser);
        parserEat(parser, TOKEN_LESS);
        var->dataType = (char*) realloc(var->dataType, (strlen(var->dataType) + strlen(parser->token->value) + 1) * sizeof(char));
        strcat(var->dataType, parser->token->value);
        parserAdvance(parser);
        parserEat(parser, TOKEN_GREATER);
    }
    else {
        var->dataType = parser->token->value;
        parserAdvance(parser);

        if(parser->token->type == TOKEN_EQUAL)
        {
            parserAdvance(parser);
            var->value = parserParseExpr(parser);
        }
    }

    return var;
}

AST_T* parserParseFnDeclaration(parser_T* parser)
{
    AST_T* fn = initAST(AST_FUNCTION);
    fn->name = parser->token->value;
    parserAdvance(parser);
    printf("Parsing function '%s'...\n", fn->name);

    parserEat(parser, TOKEN_LEFT_PAREN);

    while(parser->token->type != TOKEN_RIGHT_PAREN)
    {
        listPush(fn->children, parserParseVarDeclaration(parser));
        if(parser->token->type == TOKEN_COMMA) 
        {
            parserAdvance(parser);
        } else
        {
            break;
        }
    }
    parserEat(parser, TOKEN_RIGHT_PAREN);
    parserEat(parser, TOKEN_COLON);

    fn->dataType = parser->token->value;
    if(parser->token->type == TOKEN_VEC)
    {
        parserAdvance(parser);
        parserEat(parser, TOKEN_LESS);
        fn->dataType = (char*) realloc(fn->dataType, (strlen(fn->dataType) + strlen(parser->token->value) + 1) * sizeof(char));
        strcat(fn->dataType, parser->token->value);
        parserAdvance(parser);
        parserEat(parser, TOKEN_GREATER);
    } else
    {
        parserAdvance(parser);
    }
    parserEat(parser, TOKEN_EQUAL);
    fn->value = parserParseCompound(parser);

    return fn;
}

AST_T* parserParse(parser_T* parser)
{
    AST_T* root = initAST(AST_ROOT);

    while(parser->token->type != TOKEN_EOF) {
        switch(parser->token->type) {
            case TOKEN_FN:
                parserAdvance(parser);
                listPush(root->children, parserParseFnDeclaration(parser));
                break;
            case TOKEN_LET:
                parserAdvance(parser);
                listPush(root->children, parserParseVarDeclaration(parser));
                parserEat(parser, TOKEN_SEMICOLON);
                break;
            default:
                fprintf(stderr, "[ERROR] Parser: Unexpected token '%s'\n", tokenToString(parser->token));
                exit(1);
        }
    }

    printf("Parsing complete!\n");
    return root;
}