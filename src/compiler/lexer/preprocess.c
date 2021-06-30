#include "preprocess.h"
#include "lexer.h"
#include "token.h"
#include "../error/error.h"

#include <string.h>

typedef struct MACRO_STRUCT
{
    Token_T* tok;
    List_T* replacing_tokens;
} Macro_T;

static Macro_T* init_macro(Token_T* tok)
{
    Macro_T* mac = malloc(sizeof(struct MACRO_STRUCT));
    mac->tok = tok;
    mac->replacing_tokens = init_list(sizeof(struct TOKEN_STRUCT*));

    return mac;
}

static void free_macro(Macro_T* mac)
{
    free_list(mac->replacing_tokens);

    free_token(mac->tok);
    free(mac);
}

static Preprocessor_T* init_preprocessor(Lexer_T* lex)
{
    Preprocessor_T* pp = malloc(sizeof(struct PREPROCESSOR_STRUCT));
    pp->lex = lex;
    pp->macros = init_list(sizeof(struct MACRO_STRUCT*));
    pp->output_tokens = init_list(sizeof(struct TOKEN_STRUCT*));

    return pp;
}

static void free_preprocessor(Preprocessor_T* pp)
{
    for(size_t i = 0; i < pp->macros->size; i++)
    {
        free_macro(pp->macros->items[i]);
    }
    free_list(pp->macros);

    free(pp);
}

static inline void push_tok(Preprocessor_T* pp, Token_T* tok)
{
    list_push(pp->output_tokens, tok);
}

static Macro_T* parse_macro_def(Preprocessor_T* pp)
{
    Token_T* next = lexer_next_token(pp->lex);
    if(next->type != TOKEN_ID)
        throw_error(ERR_SYNTAX_ERROR, next, "unexpected token `%s`, expect macro name", next->value);
    Macro_T* mac = init_macro(next);

    next = lexer_next_token(pp->lex);
    if(next->type == TOKEN_LPAREN)
    {
        free(next);

        // TODO: parse arguments
        /*for(next = lexer_next_token(pp->lex); next->type != TOKEN_EOF && next->type != TOKEN_RPAREN; next = lexer_next_token(pp->lex))
        {
            if(next->type != TOKEN_ID)
                throw_error(ERR_SYNTAX_ERROR, next, "unexpected token `%s`, expect macro argument name", next->value);
            list_push(mac->args, next);

            next = lexer_next_token(pp->lex);
            if(next->type == TOKEN_RPAREN)
                break;
            if(next->type != TOKEN_COMMA) {
                throw_error(ERR_SYNTAX_ERROR, next, "unexpected token `%s`, expect `,` or `)`", next->value);
            }
            free_token(next);
        }*/
        next = lexer_next_token(pp->lex);

        if(next->type != TOKEN_RPAREN)
            throw_error(ERR_SYNTAX_ERROR, next, "unexpected token `%s`, expect `)` closing the macro argument list", next->value);
        free_token(next);
        next = lexer_next_token(pp->lex);
    }

    if(next->type != TOKEN_MACRO_BEGIN)
        throw_error(ERR_SYNTAX_ERROR, next, "unexpected token `%s`, expect `|:` after macro body", next->value);
    free_token(next);

    for(next = lexer_next_token(pp->lex); next->type != TOKEN_EOF && next->type != TOKEN_MACRO_END; next = lexer_next_token(pp->lex))
        list_push(mac->replacing_tokens, next);

    if(next->type != TOKEN_MACRO_END)
        throw_error(ERR_SYNTAX_ERROR, next, "unexpected token `%s`, expect `:|` after macro body", next->value);
    free_token(next);

    return mac;
}

static Macro_T* find_macro(Preprocessor_T* pp, char* callee)
{
    for(size_t i = 0; i < pp->macros->size; i++)
    {
        Macro_T* mac = (Macro_T*) pp->macros->items[i];
        if(strcmp(mac->tok->value, callee) == 0)
            return mac;
    }
    return NULL;
}

static void parse_macro_call(Preprocessor_T* pp, Token_T* tok)
{
    Macro_T* mac = find_macro(pp, tok->value);
    if(mac == NULL)
        throw_error(ERR_UNDEFINED, tok, "undefined macro `%s`", tok->value);

    //TODO: parse call arguments

    for(size_t i = 0; i < mac->replacing_tokens->size; i++)
        push_tok(pp, mac->replacing_tokens->items[i]);
}

List_T* lex_and_preprocess_tokens(Lexer_T* lex)
{
    Preprocessor_T* pp = init_preprocessor(lex);

    Token_T* tok;
    for(tok = lexer_next_token(lex); tok->type != TOKEN_EOF; tok = lexer_next_token(lex))
    {
        switch(tok->type)
        {
            case TOKEN_MACRO:  
                free(tok); 
                list_push(pp->macros, parse_macro_def(pp));
                break;
            case TOKEN_MACRO_CALL:
                parse_macro_call(pp, tok);
                break;
            default:
                push_tok(pp, tok);
        }
    }
    push_tok(pp, tok); // push the EOF token

    List_T* token_list = pp->output_tokens;
    free_preprocessor(pp);
    return token_list;
}