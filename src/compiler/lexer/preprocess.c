#include "preprocess.h"
#include "lexer.h"
#include "token.h"
#include "../error/error.h"
#include "../platform/platform_bindings.h"

#include <string.h>
#include <stdio.h>

#include "../io/io.h"
#include "../io/log.h"

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
    //free_token(imp->tok);
    free(imp->import_path);
    free(imp);
}

static Preprocessor_T* init_preprocessor(Lexer_T* lex)
{
    Preprocessor_T* pp = malloc(sizeof(struct PREPROCESSOR_STRUCT));
    pp->lex = lex;
    pp->macros = init_list(sizeof(struct MACRO_STRUCT*));
    pp->imports = init_list(sizeof(struct IMPORT_STRUCT*));

    pp->tokens = init_list(sizeof(struct TOKEN_STRUCT*));
    
    pp->is_silent = false;

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
    list_push(pp->tokens, tok);
}

static Macro_T* parse_macro_def(Preprocessor_T* pp, size_t* i)
{    
    (*i)++; // skip the `macro` token

    Token_T* next = pp->tokens->items[(*i)++];
    if(next->type != TOKEN_ID)
        throw_error(ERR_SYNTAX_ERROR, next, "unexpected token `%s`, expect macro name", next->value);
    Macro_T* macro = init_macro(next);

    next = pp->tokens->items[(*i)++];
    if(next->type == TOKEN_LPAREN)
    {
        free_token(next);
        next = pp->tokens->items[(*i)++];

        // TODO: evaluate arguments

        if(next->type != TOKEN_RPAREN)
            throw_error(ERR_SYNTAX_ERROR, next, "unexpected token `%s`, expect `)` after macro arguments", next->value);
        free_token(next);
        next = pp->tokens->items[(*i)++];
    }

    if(next->type != TOKEN_MACRO_BEGIN)
        throw_error(ERR_SYNTAX_ERROR, next, "unexpected token `%s`, expect `|:` to begin the macro body", next->value);
    free_token(next);

    for(next = pp->tokens->items[(*i)++]; next->type != TOKEN_EOF && next->type != TOKEN_MACRO_END; next = pp->tokens->items[(*i)++])
        list_push(macro->replacing_tokens, next);

    if(next->type != TOKEN_MACRO_END)
        throw_error(ERR_SYNTAX_ERROR, next, "unexpected token `%s`, expect `:|` to begin the macro body", next->value);
    free_token(next);

    return macro;
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
        free(full_import_path);
        free(abs_path);
        // if the file does not exist locally, search for it in the STD path

        const char* std_tmp = STD_DIR DIRECTORY_DELIMS "%s";
        char* std_path = calloc(strlen(std_tmp) + strlen(import_file->value) + 1, sizeof(char));
        sprintf(std_path, std_tmp, import_file->value);
        
        if(!file_exists(std_path))
            throw_error(ERR_SYNTAX_ERROR, import_file, "Error reading imported file \"%s\", no such file or directory", import_file->value);
        return std_path;
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

static void parse_import_def(Preprocessor_T* pp, List_T* token_list, size_t* i)
{
    // parse the import struct from the source code
    (*i)++;

    Token_T* next = token_list->items[(*i)++];
    if(next->type != TOKEN_STRING)
        throw_error(ERR_SYNTAX_ERROR, next, "unexpected token `%s`, expect `\"<import file>\"` as a string", next->value);
    Import_T* imp = init_import(next);

    next = token_list->items[(*i)];

    // generate the full path to the import
    imp->import_path = get_full_import_path((char*) imp->tok->source->path, imp->tok);

    // check if the file was already included
    if(find_import(pp, imp))
    {   
        // file is already included
        free_import(imp);
        return;
    }

    list_push(pp->imports, imp);

    // get the tokens from the file
    SrcFile_T* import_file = read_file(imp->import_path);
    import_file->short_path = strdup(imp->tok->value);
    Lexer_T* import_lexer = init_lexer(import_file);
    if(!pp->is_silent)
        LOG_OK_F(COLOR_BOLD_GREEN "  Compiling " COLOR_RESET " %s\n", imp->tok->value);

    // add the tokens
    Token_T* tok;
    for(tok = lexer_next_token(import_lexer); tok->type != TOKEN_EOF; tok = lexer_next_token(import_lexer))  
        push_tok(pp, tok);

    free_token(tok); // free the EOF token
    free_lexer(import_lexer);
    list_push(pp->files, import_file);
}

List_T* lex_and_preprocess_tokens(Lexer_T* lex, List_T* files, bool is_silent)
{
    Preprocessor_T* pp = init_preprocessor(lex);
    pp->files = files;
    pp->is_silent = is_silent;

    /**************************************
    * Stage 0: lex the main file          *
    **************************************/

    Token_T* tok;
    for(tok = lexer_next_token(lex); tok->type != TOKEN_EOF; tok = lexer_next_token(lex))
        list_push(pp->tokens, tok);
    Token_T* eof = tok;

    /**************************************
    * Stage 1: lex and import all files   *
    **************************************/

    for(size_t i = 0; i < pp->tokens->size; i++)
    {
        tok = pp->tokens->items[i];
        if(tok->type == TOKEN_IMPORT)
            parse_import_def(pp, pp->tokens, &i);
    }

    push_tok(pp, eof);

    /***************************************
    * Stage 2: parse macro definitions     *
    ***************************************/

    List_T* token_stage_2 = init_list(sizeof(struct TOKEN_STRUCT*)); // init a new list for stage 2
    for(size_t i = 0; i < pp->tokens->size;)
    {
        tok = pp->tokens->items[i];
        if(tok->type == TOKEN_MACRO)
        {
            free_token(tok);
            list_push(pp->macros, parse_macro_def(pp, &i));
            continue;
        }
        list_push(token_stage_2, tok);
        i++;
    }

    /**************************************
    * Stage 3: expand all macro calls     *
    **************************************/

    List_T* token_stage_3 = init_list(sizeof(struct TOKEN_STRUCT*)); // init a new list for stage 3
    for(size_t i = 0; i < token_stage_2->size; i++)
    {
        tok = token_stage_2->items[i];
        if(tok->type == TOKEN_MACRO_CALL)
        {
            Macro_T* macro = find_macro(pp, tok->value);
            //free_token(tok);
            if(!macro)
                throw_error(ERR_UNDEFINED, tok, "unedefined macro `%s`", tok->value);
            
            free_token(tok);

            for(size_t i = 0; i < macro->replacing_tokens->size; i++)
                list_push(token_stage_3, macro->replacing_tokens->items[i]);
            continue;
        }
        list_push(token_stage_3, tok);
    }

    free_list(pp->tokens);
    free_list(token_stage_2);
    free_preprocessor(pp);
    return token_stage_3;
}