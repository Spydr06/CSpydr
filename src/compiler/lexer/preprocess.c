#include "preprocess.h"
#include "lexer.h"
#include "token.h"
#include "../error/error.h"
#include "../platform/platform_bindings.h"

#include <string.h>
#include <stdio.h>

#include "../io/io.h"

typedef struct MACRO_STRUCT
{
    Token_T* tok;
    List_T* replacing_tokens;
} Macro_T;

typedef struct IMPORT_STRUCT
{
    Token_T* tok;
    char* import_path;
} Import_T;

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

static Import_T* init_import(Token_T* tok)
{
    Import_T* imp = malloc(sizeof(struct IMPORT_STRUCT));
    imp->tok = tok;

    return imp;
}

static void free_import(Import_T* imp)
{
    free_token(imp->tok);
    free(imp->import_path);
    free(imp);
}

static Preprocessor_T* init_preprocessor(Lexer_T* lex)
{
    Preprocessor_T* pp = malloc(sizeof(struct PREPROCESSOR_STRUCT));
    pp->lex = lex;
    pp->macros = init_list(sizeof(struct MACRO_STRUCT*));
    pp->output_tokens = init_list(sizeof(struct TOKEN_STRUCT*));
    pp->imports = init_list(sizeof(struct IMPORT_STRUCT*));

    return pp;
}

static void free_preprocessor(Preprocessor_T* pp)
{
    for(size_t i = 0; i < pp->macros->size; i++)
        free_macro(pp->macros->items[i]);
    free_list(pp->macros);

    for(size_t i = 0; i < pp->imports->size; i++)
        free_import(pp->imports->items[i]);
    free_list(pp->imports);

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
        free_token(next);

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


static char* get_full_import_path(char* origin, Token_T* import_file)
{
    // first get the full directory of the origin
    char* abs_path = get_absolute_path(origin);
    char* full_path = get_path_from_file(abs_path);

    // construct the imported file onto it
    const char* template = "%s" DIRECTORY_DELIMS "%s";
    char* full_import_path = calloc(strlen(template) + strlen(full_path) + strlen(import_file->value) + 1, sizeof(char));
    sprintf(full_import_path, template, full_path, import_file->value);

    if(!file_exists(full_import_path))
    {
        throw_error(ERR_SYNTAX_ERROR, import_file, "Error reading imported file \"%s\", no such file or directory", import_file->value);
    }

    free(abs_path);

    return full_import_path;
}

static Import_T* find_import(Preprocessor_T* pp, Import_T* imp)
{
    for(size_t i = 0; i < pp->imports->size; i++)
    {
        Import_T* b_imp = pp->imports->items[i];
        if(strcmp(b_imp->import_path, imp->import_path) == 0)
            return b_imp;
    }
    return NULL;
}

static void parse_import_def(Preprocessor_T* pp)
{
    // parse the import struct from the source code
    Token_T* next = lexer_next_token(pp->lex);
    if(next->type != TOKEN_STRING)
        throw_error(ERR_SYNTAX_ERROR, next, "unexpected token `%s`, expect `\"<import file>\"` as a string", next->value);
    
    Import_T* imp = init_import(next);

    next = lexer_next_token(pp->lex);
    if(next->type != TOKEN_SEMICOLON)
        throw_error(ERR_SYNTAX_ERROR, next, "unexpected token `%s`, expect `;` after import file", next->value);
    free_token(next);

    // generate the full path to the import
    imp->import_path = get_full_import_path((char*) pp->lex->file->path, imp->tok);

    // check if the file was already included
    if(find_import(pp, imp))
    {   
        printf("Path %s is used already\n", imp->import_path);
        // file is already included
        free_import(imp);
        return;
    }

    list_push(pp->imports, imp);

    // get the tokens from the file
    SrcFile_T* import_file = read_file(imp->import_path);
    Lexer_T* import_lexer = init_lexer(import_file);

    // add the tokens
    for(Token_T* tok = lexer_next_token(import_lexer); tok->type != TOKEN_EOF; tok = lexer_next_token(import_lexer))  
        list_push(pp->output_tokens, tok);

    free_lexer(import_lexer);
    // TODO: free_srcfile(import_file);
}

List_T* lex_and_preprocess_tokens(Lexer_T* lex)
{
    Preprocessor_T* pp = init_preprocessor(lex);

    Token_T* tok;
    for(tok = lexer_next_token(lex); tok->type != TOKEN_EOF; tok = lexer_next_token(lex))
    {
        switch(tok->type)
        {
            case TOKEN_IMPORT:
                free(tok);
                parse_import_def(pp);
                break;
            default:
                push_tok(pp, tok);
        }
    }
    push_tok(pp, tok); // push the EOF token

    List_T* token_list_pre_macro_init = pp->output_tokens;
    pp->output_tokens = init_list(sizeof(struct TOKEN_STRUCT*));
    for(size_t i = 0; i < token_list_pre_macro_init->size; i++)
    {
        Token_T* tok = token_list_pre_macro_init->items[i];
        if(tok->type == TOKEN_MACRO)
        {
            free(tok);
            list_push(pp->macros, parse_macro_def(pp));
        }
        else
            push_tok(pp, tok);
    }
    free_list(token_list_pre_macro_init);

    List_T* token_list_pre_macro_eval = pp->output_tokens;
    pp->output_tokens = init_list(sizeof(struct TOKEN_STRUCT*));
    for(size_t i = 0; i < token_list_pre_macro_eval->size; i++)
    {
        Token_T* tok = token_list_pre_macro_eval->items[i];
        if(tok->type == TOKEN_MACRO_CALL)
        {
            free(tok);
            parse_macro_call(pp, tok);
        }
        else
            push_tok(pp, tok);
    }
    free_list(token_list_pre_macro_eval);

    List_T* token_list = pp->output_tokens;
    free_preprocessor(pp);
    return token_list;
}