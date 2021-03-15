#include "parser.h"
#include "list.h"
#include "token.h"
#include "log.h"

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
        LOG_ERROR("Unexpected token '%s' in line %d.\n%s\n", parser->token->value, parser->token->line, msg);
    }

    return parserAdvance(parser);
}

static AST_T* parserParseFnDef(parser_T* parser);
static AST_T* parserParseCompound(parser_T* parser);
static AST_T* parserParseVarDef(parser_T* parser);

AST_T* parserParse(parser_T* parser)
{
    AST_T* root = initAST(ROOT, 0);
    root->root->contents = initList(sizeof(struct AST_STRUCT*));

    while(parser->token->type != TOKEN_EOF)
    {
        switch(parser->token->type)
        {
            case TOKEN_FN:
                parserAdvance(parser);
                listPush(root->root->contents, parserParseFnDef(parser));
                break;
            case TOKEN_LET:
                parserAdvance(parser);
                parserParseVarDef(parser);
                parserConsume(parser, TOKEN_SEMICOLON, "Expect ';' after variable definition.");
                break;
            default:
                LOG_ERROR("Unexpected token '%s' in line %d.\n", parser->token->value, parser->token->line);
                exit(1);
        }
    }

    LOG_OK("Parsing complete!%s", "\n");
    return root;
}

static AST_T* parserParseCompound(parser_T* parser)
{
    AST_T* cmp = initAST(COMPOUND, 0);
    cmp->compound->contents = initList(sizeof(struct AST_STRUCT*));

    parserConsume(parser, TOKEN_LEFT_BRACE, "Expect '{'.");

    while(parser->token->type != TOKEN_RIGHT_BRACE)
    {
        //TODO: parse code
        if(parser->token->type == TOKEN_EOF)
        {
            parserConsume(parser, TOKEN_RIGHT_BRACE, "Unterminated compound, expect '}'.");
            exit(1);
        }

        parserAdvance(parser);
    }

    parserConsume(parser, TOKEN_RIGHT_BRACE, "Expect '}'.");

    return cmp;
}

static AST_T* parserParseVarDef(parser_T* parser)
{
    AST_T* var = initAST(DEF, VAR);
    var->def->name = parser->token->value;
    parserConsume(parser, TOKEN_ID, "Expect variable name.");

    var->def->isFunction = false;
    parserConsume(parser, TOKEN_COLON, "Expect ':' after variable name.");
    var->def->dataType = parser->token->value;
    parserConsume(parser, TOKEN_ID, "Expect data type after ':'.");

    if(parser->token->type == TOKEN_EQUALS)
    {
        // TODO: parse assignment
        parserAdvance(parser);
        parserAdvance(parser);
    }

    return var;
}

static AST_T* parserParseFnDef(parser_T* parser)
{
    AST_T* fn = initAST(DEF, FN);
    fn->def->name = parser->token->value;
    parserConsume(parser, TOKEN_ID, "Expect function name.");

    fn->def->isFunction = true;
    fn->def->args = initList(sizeof(struct AST_STRUCT));

    parserConsume(parser, TOKEN_LEFT_PAREN, "Expect '(' after function name.");
    
    while(parser->token->type != TOKEN_RIGHT_PAREN)
    {
        // TODO: parse arg list
        parserAdvance(parser);
    }

    parserConsume(parser, TOKEN_RIGHT_PAREN, "Expect ')' after function arguments.");

    if(parser->token->type == TOKEN_COLON)
    {
        parserAdvance(parser);
        fn->def->dataType = parser->token->value;
        parserConsume(parser, TOKEN_ID, "Expect return type after ':'.");
    }

    parserConsume(parser, TOKEN_EQUALS, "Expect '=' after function definition.");

    fn->def->value = parserParseCompound(parser);

    return fn;    
}