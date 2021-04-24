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
static ASTCompound_T* parserParseCompound(parser_T* parser);

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

static ASTLoop_T* parserParseLoop(parser_T* parser) // TODO: loops will for now only support while-like syntax; for and foreach come soon
{
    parserConsume(parser, TOKEN_LOOP, "expect `loop` keyword");

    ASTExpr_T* condition = parserParseExpr(parser, LOWEST);
    ASTCompound_T* body = parserParseCompound(parser);

    return initASTLoop(condition, body);
}

static ASTMatch_T* parserParseMatch(parser_T* parser)
{
    parserConsume(parser, TOKEN_MATCH, "expect `match` keyword");
    
    ASTExpr_T* condition = parserParseExpr(parser, LOWEST);
    parserConsume(parser, TOKEN_LBRACE, "expect `{` after match condtion");

    list_T* cases = initList(sizeof(struct AST_EXPRESSION_STRUCT*));
    list_T* bodys = initList(sizeof(struct AST_COMPOUND_STRUCT*));
    ASTCompound_T* defaultBody = NULL;
    while(!tokIs(parser, TOKEN_RBRACE))
    {   
        switch(parser->tok->type)
        {
            case TOKEN_UNDERSCORE:
                if(defaultBody == NULL) {
                    parserConsume(parser, TOKEN_UNDERSCORE, "expect `_` for default case");
                    parserConsume(parser, TOKEN_ARROW, "expect `=>` after match case");
                    defaultBody = parserParseCompound(parser);
                } else
                {
                    throwRedefinitionError(parser->eh, "redefinition of default match case", parser->tok->line, parser->tok->pos);
                    exit(1);
                }
                break;

            case TOKEN_EOF:
                throwSyntaxError(parser->eh, "expect '}' after match statement", parser->tok->line, parser->tok->pos);
                exit(1);
                break;

            default:
                listPush(cases, parserParseExpr(parser, LOWEST));
                parserConsume(parser, TOKEN_ARROW, "expect `=>` after match case");
                listPush(bodys, parserParseCompound(parser));
                break;
        }
    }
    parserAdvance(parser);

    return initASTMatch(condition, cases, bodys, defaultBody);
}

static ASTIf_T* parserParseIf(parser_T* parser)
{
    parserConsume(parser, TOKEN_IF, "expect `if` keyword");
    ASTExpr_T* condition = parserParseExpr(parser, LOWEST);
    ASTCompound_T* ifBody = parserParseCompound(parser);
    ASTCompound_T* elseBody = NULL;

    if(tokIs(parser, TOKEN_ELSE)) {
        parserAdvance(parser);
        elseBody = parserParseCompound(parser);
    }
    
    return initASTIf(condition, ifBody, elseBody);
}

static ASTReturn_T* parserParseReturn(parser_T* parser)
{
    parserConsume(parser, TOKEN_RETURN, "expect `ret` keyword");
    ASTReturn_T* ast = initASTReturn(parserParseExpr(parser, LOWEST));
    parserConsume(parser, TOKEN_SEMICOLON, "expect `;` after return value");
    return ast;
}

static ASTLocal_T* parserParseLocal(parser_T* parser)
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

    ASTLocal_T* ast = initASTLocal(type, value, name);
    free(name);
    return ast;
}

static ASTExprStmt_T* parserParseExpressionStatement(parser_T* parser)
{
    ASTExprStmt_T* ast = initASTExprStmt(parserParseExpr(parser, LOWEST));
    parserConsume(parser, TOKEN_SEMICOLON, "expect `;` after expression");
    return ast;
}

static ASTStmt_T* parserParseStatement(parser_T* parser)
{
    void* stmt;
    ASTStmtType_T type;

    switch(parser->tok->type)
    {
        case TOKEN_LOOP:
            stmt = parserParseLoop(parser);
            type = STMT_LOOP;
            break;
        
        case TOKEN_MATCH:
            stmt = parserParseMatch(parser);
            type = STMT_MATCH;
            break;

        case TOKEN_IF:
            stmt = parserParseIf(parser);
            type = STMT_IF;
            break;

        case TOKEN_RETURN:
            type = STMT_RETURN;
            stmt = parserParseReturn(parser);
            break;

        case TOKEN_LET:
            type = STMT_LET;
            stmt = parserParseLocal(parser);
            break;

        case TOKEN_ID:
            type = STMT_EXPRESSION;
            stmt = parserParseExpressionStatement(parser);
            break;
        
        default:
            throwSyntaxError(parser->eh, "expect statement", parser->tok->line, parser->tok->pos);
            exit(1);
            break;
    }

    return initASTStmt(type, stmt);
}

static ASTCompound_T* parserParseCompound(parser_T* parser)
{
    list_T* stmts = initList(sizeof(struct AST_STATEMENT_STRUCT*));

    if(tokIs(parser, TOKEN_LBRACE))
    {
        parserAdvance(parser);
        
        while(!tokIs(parser, TOKEN_RBRACE))
        {
            listPush(stmts, parserParseStatement(parser));

            if(tokIs(parser, TOKEN_EOF))
            {
                throwSyntaxError(parser->eh, "expect '}' after compound", parser->tok->line, parser->tok->pos);
                exit(1);
            }
        }
        parserAdvance(parser);
    } else  // enables to do single statement compounds without braces
    {
        listPush(stmts, parserParseStatement(parser));
    }

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