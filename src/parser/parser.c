#include "parser.h"

#include <string.h>

parser_T* initParser(lexer_T* lexer)
{
    parser_T* parser = calloc(1, sizeof(struct PARSER_STRUCT));
    parser->lexer = lexer;
    parser->eh = parser->lexer->eh;
    parser->localVars = initList(sizeof(struct AST_LOCAL_STRUCT*));
    parser->tok = lexerNextToken(parser->lexer);

    return parser;
}

void freeParser(parser_T* parser)
{
    freeList(parser->localVars);
    freeToken(parser->tok);
    free(parser);
}

token_T* parserAdvance(parser_T* parser)
{
    freeToken(parser->tok);
    parser->tok = lexerNextToken(parser->lexer);
    return parser->tok;
}

bool tokIs(parser_T* parser, tokenType_T type)
{
    return parser->tok->type == type;
}

bool streq(char* s1, char* s2)
{
    return strcmp(s1, s2) == 0;
}

token_T* parserConsume(parser_T* parser, tokenType_T type, const char* msg)
{
    if(!tokIs(parser, type))
    {
        throwSyntaxError(parser->eh, msg, parser->tok->line, parser->tok->pos);
    }

    return parserAdvance(parser);
}

static ASTGlobal_T* parserParseGlobal(parser_T* parser);
static ASTFunction_T* parserParseFunction(parser_T* parser);
static ASTImport_T* parserParseImport(parser_T* parser);
static ASTTypedef_T* parserParseTypedef(parser_T* parser);

ASTRoot_T* parserParse(parser_T* parser)
{
    ASTRoot_T* root = initASTRoot(parser->lexer->file->path);

    while(!tokIs(parser, TOKEN_EOF))
    {
        switch(parser->tok->type)
        {
            case TOKEN_LET:
                listPush(root->globals, parserParseGlobal(parser));
                break;
            case TOKEN_FN:
                listPush(root->functions, parserParseFunction(parser));
                break;
            case TOKEN_IMPORT:
                listPush(root->imports, parserParseImport(parser));
                break;
            case TOKEN_TYPE:
                listPush(root->types, parserParseTypedef(parser));
                break;

            default:
                throwSyntaxError(parser->eh, "unexpected token", parser->tok->line, parser->tok->pos);
                break;
        }
    }

    return root;
}

static ASTType_T* parserParseType(parser_T* parser)
{
    char* type = parser->tok->value;
    ASTDataType_T dt = AST_STRUCT;

    if(streq(type, "i32"))
        dt = AST_I32;
    else if(streq(type, "i64"))
        dt = AST_I64;
    else if(streq(type, "u32"))
        dt = AST_U32;
    else if(streq(type, "u64"))
        dt = AST_U64;
    else if(streq(type, "f32"))
        dt = AST_F32;
    else if(streq(type, "f64"))
        dt = AST_F64;
    else if(streq(type, "bool"))
        dt = AST_BOOL;
    else if(streq(type, "char"))
        dt = AST_CHAR;
    else if(streq(type, "bool"))
        dt = AST_BOOL;
    else if(streq(type, "char"))
        dt = AST_CHAR;  
    else if(streq(type, "str"))
        dt = AST_STRING;

    parserConsume(parser, TOKEN_ID, "expect data type");

    return initASTType(dt, NULL);
}

static ASTExpr_T* parserParseExpr(parser_T* parser, precedence_T precedence)
{

}

static ASTCompound_T* parserParseCompound(parser_T* parser)
{
    parserConsume(parser, TOKEN_LBRACE, "expect `{` before compound");
    list_T* stmts = initList(sizeof(struct AST_STATEMENT_STRUCT*));
    parserConsume(parser, TOKEN_RBRACE, "expect `}` after compound");

    return initASTCompound(stmts);
}

static ASTGlobal_T* parserParseGlobal(parser_T* parser)
{
    parserConsume(parser, TOKEN_LET, "expect `let` keyword");
    char* name = strdup(parser->tok->value);
    parserConsume(parser, TOKEN_ID, "expect variable name");
    parserConsume(parser, TOKEN_COLON, "expect `:` after variable name");

    ASTType_T* type = parserParseType(parser);

    ASTExpr_T* value = NULL;
    if(tokIs(parser, TOKEN_ASSIGN))
        value = parserParseExpr(parser, LOWEST);
    
    parserConsume(parser, TOKEN_SEMICOLON, "expect `;` after variable definition");

    ASTGlobal_T* ast = initASTGlobal(name, type, value);
    free(name);
    return ast;
}

static ASTFunction_T* parserParseFunction(parser_T* parser)
{
    parserConsume(parser, TOKEN_FN, "expect `fn` keyword");
    char* name = strdup(parser->tok->value);
    parserConsume(parser, TOKEN_ID, "expect function name");
    parserConsume(parser, TOKEN_LPAREN, "expect `(` after function name");

    list_T* args = initList(sizeof(struct AST_ARGUMENT_STRUCT*));
    while(!tokIs(parser, TOKEN_RPAREN))
    {
        char* argName = strdup(parser->tok->value);
        parserConsume(parser, TOKEN_ID, "expect argument name");
        parserConsume(parser, TOKEN_COLON, "expect `:` after argument name");
        listPush(args, initASTArgument(argName, parserParseType(parser)));
        free(argName);

        if(!tokIs(parser, TOKEN_RPAREN))
            parserConsume(parser, TOKEN_COMMA, "expect `,` between arguments");
    }
    parserAdvance(parser);
    
    ASTType_T* returnType = NULL;
    if(tokIs(parser, TOKEN_COLON))
        returnType = parserParseType(parser);
    else {
        returnType = initASTType(AST_VOID, NULL);
    }

    ASTCompound_T* body = parserParseCompound(parser);

    ASTFunction_T* ast = initASTFunction(name, returnType, body, args);
    free(name);
    return ast;
}

static ASTImport_T* parserParseImport(parser_T* parser)
{
    parserConsume(parser, TOKEN_IMPORT, "expect `import` keyword");
    ASTImport_T* ast = initASTImport(parser->tok->value);
    parserConsume(parser, TOKEN_STRING, "expect filepath to import");
    parserConsume(parser, TOKEN_SEMICOLON, "expect `;` after import");

    return ast;
}

static ASTTypedef_T* parserParseTypedef(parser_T* parser)
{
    parserConsume(parser, TOKEN_TYPE, "expect `type` keyword");
    char* name = strdup(parser->tok->value);
    parserConsume(parser, TOKEN_ID, "expect type name");
    parserConsume(parser, TOKEN_COLON, "expect `:` after typename");

    ASTTypedef_T* ast = initASTTypedef(parserParseType(parser), name);
    free(name);

    parserConsume(parser, TOKEN_SEMICOLON, "expect `;` after type defintion");
    return ast;
}