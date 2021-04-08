#include "parser.h"
#include "AST.h"

#include "stmtParser.h"
#include "exprParser.h"

#include <string.h>
#include <stdio.h>

#define SYNTAX_ERROR(msg) throwSyntaxError(parser->lexer->errorHandler, msg, parser->lexer->srcPath, parser->lexer->line, parser->lexer->iInLine)

parser_T* initParser(lexer_T* lexer)
{
    parser_T* parser = calloc(1, sizeof(struct PARSER_STRUCT));
    parser->lexer = lexer;
    parser->token = lexerNextToken(lexer);

    return parser;
}

token_T* parserAdvance(parser_T* parser)
{
    parser->token = lexerNextToken(parser->lexer);
    return parser->token;
}

token_T* parserConsume(parser_T* parser, tokenType_T type, const char* msg)
{
    if(parser->token->type != type)
    {
        SYNTAX_ERROR(msg);
        exit(1);
    }

    return parserAdvance(parser);
}

ASTDataType_T* parserParseDataType(parser_T* parser)
{
    ASTBasicDataType_T basicType = AST_UNDEF;
    ASTDataType_T* subType = NULL;
    ASTExpr_T* numberOfIndices = NULL;

    char* type = parser->token->value;
    if(strcmp(type, "i32") == 0)
    {
        basicType = AST_I32;
        parserAdvance(parser);
    }
    else if(strcmp(type, "i64") == 0)
    {
        basicType = AST_I64;
        parserAdvance(parser);
    }
    else if(strcmp(type, "u32") == 0)
    {
        basicType = AST_U32;
        parserAdvance(parser);
    }
    else if(strcmp(type, "u64") == 0)
    {
        basicType = AST_U64;
        parserAdvance(parser);
    }
    else if(strcmp(type, "f32") == 0)
    {
        basicType = AST_F32;
        parserAdvance(parser);
    }
    else if(strcmp(type, "f64") == 0)
    {
        basicType = AST_F64;
        parserAdvance(parser);
    }
    else if(strcmp(type, "bool") == 0)
    {
        basicType = AST_BOOL;
        parserAdvance(parser);
    }
    else if(strcmp(type, "char") == 0)
    {
        basicType = AST_CHAR;
        parserAdvance(parser);
    }
    else if(strcmp(type, "str") == 0)
    {
        basicType = AST_STR;
        parserAdvance(parser);
    }
    else if(strcmp(type, "[") == 0)
    {
        parserAdvance(parser);
        subType = parserParseDataType(parser);
        basicType = AST_ARRAY;

        if(parser->token->type == TOKEN_COLON)
        {
            parserAdvance(parser);
            numberOfIndices = parserParseExpr(parser);
        }
        parserConsume(parser, TOKEN_RIGHT_BRACKET, "expect \"]\" after array type declaration");
    }
    else 
    {
        SYNTAX_ERROR("expect data type (e.g. i32, f32, bool, str)");
        exit(1);
    }

    ASTDataType_T* ast = initASTDataType_T(basicType);
    ast->innerType = subType;
    ast->numberOfIndices = numberOfIndices;
    return ast;
}

static ASTCompoundInstruction_T* parserParseCallableId(parser_T* parser)
{
    char* id = parser->token->value;
    parserConsume(parser, TOKEN_ID, "expect function or variable call");

    if(parser->token->type == TOKEN_LEFT_PAREN)
    {
        ASTExprFnCall_T* ast = parserParseFunctionCall(parser, id);

        parserConsume(parser, TOKEN_SEMICOLON, "expect \";\" after function call");
        return initCompoundInstruction(AST_CI_FN_CALL, ast);
    }
    else 
    {
        ASTAssignment_T* ast = parserParseAssinment(parser, id);
        parserConsume(parser, TOKEN_SEMICOLON, "expect \";\" after assignment");
        return initCompoundInstruction(AST_CI_ASSIGN, ast);
    }
}

static ASTCompoundInstruction_T* parserParseCompoundInstruction(parser_T* parser, ASTCompound_T* compound)
{
    ASTCompoundInstructionType_T type;
    void* ptr = NULL;

    switch(parser->token->type)
    {
        case TOKEN_STMT: {
            if(strcmp(parser->token->value, "if") == 0)
            {
                type = AST_CI_IF_STMT;
                ptr = parserParseIf(parser);
            }
            if(strcmp(parser->token->value, "for") == 0)
            {
                type = AST_CI_FOR_STMT;
                parserParseFor(parser);
            }
            if(strcmp(parser->token->value, "while") == 0)
            {
                type = AST_CI_WHILE_STMT;
                parserParseWhile(parser);
            }
            if(strcmp(parser->token->value, "exit") == 0)
            {
                type = AST_CI_EXIT_STMT;
                parserParseExitStmt(parser);
            }
            if(strcmp(parser->token->value, "return") == 0)
            {
                type = AST_CI_RETURN_STMT;
                parserParseReturnStmt(parser);
            }
        } break;

        case TOKEN_ID:
            return parserParseCallableId(parser);

        case TOKEN_LET:
            ptr = (void*) parserParseLocal(parser, true);
            parserConsume(parser, TOKEN_SEMICOLON, "expect \";\" after variable definition");

            type = AST_CI_LOCALDEF;
            break;

        default: {
            const char* template = "unexpected token \"%s\"; expect expression";
            char* errorMsg = calloc(strlen(template) + strlen(parser->token->value) + 1, sizeof(char));

            sprintf(errorMsg, template, parser->token->value);
            SYNTAX_ERROR(errorMsg);
            exit(1);
        }
    }

    return initCompoundInstruction(type, ptr);
}

ASTCompound_T* parserParseCompound(parser_T* parser)
{
    ASTCompound_T* ast = initASTCompound();

    if(parser->token->type == TOKEN_LEFT_BRACE)
    {
        parserAdvance(parser);
        while(parser->token->type != TOKEN_RIGHT_BRACE)
        {
            listPush(ast->body, parserParseCompoundInstruction(parser, ast));
            if(parser->token->type == TOKEN_EOF)
            {
                SYNTAX_ERROR("unclosed compound at EOF; expect \")\"");
            }
        }

        parserConsume(parser, TOKEN_RIGHT_BRACE, "expect \"}\" after compund");
    }
    else
    {
        listPush(ast->body, parserParseCompoundInstruction(parser, ast));
    } 

    return ast;
}

static ASTArgument_T* parserParseArgument(parser_T* parser)
{
    char* name = parser->token->value;
    parserConsume(parser, TOKEN_ID, "expect argument name");
    parserConsume(parser, TOKEN_COLON, "expect \";\" after argument name");

    return initASTArgument(name, parserParseDataType(parser));
}

static ASTFunction_T* parserParseFunction(parser_T* parser)
{
    ASTFunction_T* ast = initASTFunction(parser->token->value);
    parserConsume(parser, TOKEN_ID, "expect function name");
    parserConsume(parser, TOKEN_LEFT_PAREN, "expect \"(\" after function name");

    while(parser->token->type != TOKEN_RIGHT_PAREN)
    {
        listPush(ast->args, parserParseArgument(parser));

        if(parser->token->type != TOKEN_RIGHT_PAREN)
        {
            parserConsume(parser, TOKEN_COMMA, "expect \",\" between arguments or \")\" after arguments");
        }
    }
    parserConsume(parser, TOKEN_RIGHT_PAREN, "expect \")\" after function arguments");

    if(parser->token->type == TOKEN_COLON)
    {
        parserAdvance(parser);
        ast->returnType = parserParseDataType(parser);
    } else
    {
        ast->returnType = initASTDataType_T(AST_VOID);
    }

    parserConsume(parser, TOKEN_EQUALS_GREATER, "expect \"=>\" after function definition");

    ast->body = parserParseCompound(parser);

    return ast;
}

static ASTGlobal_T* parserParseGlobal(parser_T* parser)
{
    char* name = parser->token->value;
    parserConsume(parser, TOKEN_ID, "expect global name");
    parserConsume(parser, TOKEN_COLON, "expect \":\" after global name");

    ASTDataType_T* dataType = parserParseDataType(parser);
    ASTExpr_T* value = NULL;

    if(parser->token->type == TOKEN_EQUALS)
    {
        parserAdvance(parser);
        value = parserParseExpr(parser);
    }

    return initASTGlobal(name, dataType, value);
}

ASTRoot_T* parserParse(parser_T* parser)
{
    ASTRoot_T* ast = initASTRoot();

    while(parser->token->type != TOKEN_EOF)
    {
        switch(parser->token->type)
        {
            case TOKEN_FN:
                parserAdvance(parser);
                listPush(ast->functions, parserParseFunction(parser));
                break;
            case TOKEN_LET:
                parserAdvance(parser);
                listPush(ast->globals, parserParseGlobal(parser));
                parserConsume(parser, TOKEN_SEMICOLON, "expect \";\" after global definition");
                break;
            
            default: {
                const char* template = "unexpected token \"%s\"";
                char* errorMsg = calloc(strlen(template) + strlen(parser->token->value) + 1, sizeof(char));

                sprintf(errorMsg, template, parser->token->value);
                SYNTAX_ERROR(errorMsg);
                exit(1);
            }
        }
    }

    return ast;
}