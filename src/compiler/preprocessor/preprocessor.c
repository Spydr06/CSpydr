#include "preprocessor.h"
#include "config.h"
#include "stdmacros.h"
#include "../lexer/lexer.h"
#include "../lexer/token.h"
#include "../error/error.h"
#include "../platform/platform_bindings.h"
#include "../globals.h"

#include <string.h>
#include <stdio.h>
#include <stdint.h>

#include "../io/io.h"
#include "../io/log.h"

#define throw_error(...)              \
    do {                              \
        fprintf(OUTPUT_STREAM, "\n"); \
        throw_error(__VA_ARGS__);     \
    } while(0)

//
// Base structs
//

typedef struct PREPROCESSOR_STRUCT 
{
    Lexer_T* lex;
    List_T* files;

    List_T* tokens;
    List_T* macros;
    List_T* imports;

    bool is_silent;
} Preprocessor_T;

typedef struct MACRO_CALL_STRUCT
{
    Token_T* tok;
    Macro_T* macro;

    u8 argc;
    struct { size_t start_idx, end_idx; } args[__CSP_MAX_FN_NUM_ARGS];
} __attribute__((packed)) MacroCall_T;

typedef struct IMPORT_STRUCT
{
    Token_T* tok;
    char* import_path;
} __attribute__((packed)) Import_T;

//
// Base functions
//

Macro_T* init_macro(Token_T* tok)
{
    Macro_T* mac = malloc(sizeof(struct MACRO_STRUCT));
    mac->tok = tok;
    mac->replacing_tokens = init_list(sizeof(struct TOKEN_STRUCT*));
    mac->argc = 0;
    mac->used = false;

    return mac;
}

static Macro_T* find_macro(Preprocessor_T* pp, char* callee, u8 argc);

static void init_macro_call(Preprocessor_T* pp, MacroCall_T* call, Token_T* tok)
{
    call->tok = tok;

    call->argc = 0;
    memset(call->args, 0, sizeof(*call->args));
}

static void free_macro(Macro_T* mac)
{
    free_list(mac->replacing_tokens);
    /*for(size_t i = 0; i < mac->replacing_tokens->size; i++)
        free_token(mac->replacing_tokens->items[i]);*/
    free(mac);
}

static bool macro_has_arg(Macro_T* macro, char* callee)
{
    for(u8 i = 0; i < macro->argc; i++)
        if(strcmp(callee, macro->args[i]->value) == 0)
            return true;
    return false;
}

static Import_T* init_import(Token_T* tok)
{
    Import_T* imp = malloc(sizeof(struct IMPORT_STRUCT));
    imp->tok = tok;

    return imp;
}

static void free_import(Import_T* imp)
{
    free(imp->import_path);
    free(imp);
}
 
void init_preprocessor(Preprocessor_T* pp, Lexer_T* lex)
{
    pp->lex = lex;
    pp->macros = init_list(sizeof(struct MACRO_STRUCT*));
    pp->imports = init_list(sizeof(struct IMPORT_STRUCT*));

    pp->tokens = init_list(sizeof(struct TOKEN_STRUCT*));
    
    pp->is_silent = false;

    define_std_macros(pp->macros);
}

static void free_preprocessor(Preprocessor_T* pp)
{
    for(size_t i = 0; i < pp->macros->size; i++)
        free_macro(pp->macros->items[i]);
    free_list(pp->macros);

    for(size_t i = 0; i < pp->imports->size; i++)
        free_import(pp->imports->items[i]);
    free_list(pp->imports);
}

static inline void push_tok(Preprocessor_T* pp, Token_T* tok)
{
    list_push(pp->tokens, tok);
}

//
// Import parsing and lexing
//

static char* get_full_import_path(char* origin, Token_T* import_file)
{
    // first get the full directory of the origin
    char* abs_path = get_absolute_path(origin);
    char* full_path = get_path_from_file(abs_path);

    // construct the imported file onto it
    const char* template = "%s" DIRECTORY_DELIMS "%s";
    char* full_import_path = calloc(strlen(template) + strlen(full_path) + strlen(import_file->value) + 1, sizeof(char));
    sprintf(full_import_path, template, full_path, import_file->value);

    free(abs_path);

    if(!file_exists(full_import_path))
    {
        free(full_import_path);
        // if the file does not exist locally, search for it in the STD path

        const char* std_tmp = STD_DIR DIRECTORY_DELIMS "%s";
        char* std_path = calloc(strlen(std_tmp) + strlen(import_file->value) + 1, sizeof(char));
        sprintf(std_path, std_tmp, import_file->value);
        
        if(!file_exists(std_path))
            throw_error(ERR_SYNTAX_ERROR, import_file, "Error reading imported file \"%s\", no such file or directory", import_file->value);
        return std_path;
    }

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
    static u32 file_no = 0;
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
    import_file->file_no = ++file_no;

    Lexer_T import_lexer;
    init_lexer(&import_lexer, import_file);
    
    if(!pp->is_silent) {
        LOG_OK_F("\33[2K\r" COLOR_BOLD_GREEN "  Compiling " COLOR_RESET " %s", imp->tok->value);
        fflush(OUTPUT_STREAM);
    }

    // add the tokens
    Token_T* tok;
    for(tok = lexer_next_token(&import_lexer); tok->type != TOKEN_EOF; tok = lexer_next_token(&import_lexer))  
        push_tok(pp, tok);
    
    list_push(pp->files, import_file);
}

//
// Macro definition parsing
//

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
        for(next = pp->tokens->items[(*i)++]; next->type != TOKEN_EOF && next->type != TOKEN_RPAREN; next = pp->tokens->items[(*i)++]) 
        {
            if(next->type != TOKEN_ID)
                throw_error(ERR_SYNTAX_ERROR, next, "unexpected token `%s`, expect macro argument name", next->value);
            if(macro_has_arg(macro, next->value))
                throw_error(ERR_REDEFINITION, next, "duplicate macro argument `%s`", next->value);
            if(macro->argc >= __CSP_MAX_FN_NUM_ARGS)
                throw_error(ERR_MISC, next, "too many macro arguments, maximal argument count is `%d`", __CSP_MAX_FN_NUM_ARGS);

            macro->args[macro->argc++] = next;

            next = pp->tokens->items[(*i)++];
            if(next->type == TOKEN_RPAREN)
                break;

            if(next->type != TOKEN_COMMA)
                throw_error(ERR_SYNTAX_ERROR, next, "unexpected token `%s`, expect `,` between macro arguments", next->value);
        }

        if(next->type != TOKEN_RPAREN)
            throw_error(ERR_SYNTAX_ERROR, next, "unexpected token `%s`, expect `)` after macro arguments", next->value);

        next = pp->tokens->items[(*i)++];
    }

    if(next->type != TOKEN_LBRACE)
        throw_error(ERR_SYNTAX_ERROR, next, "unexpected token `%s`, expect `{` to begin the macro body", next->value);

    size_t depth = 0;
    for(next = pp->tokens->items[(*i)++]; next->type != TOKEN_EOF; next = pp->tokens->items[(*i)++])
    {
        if(next->type == TOKEN_LBRACE)
            depth++;
        else if(next->type == TOKEN_RBRACE)
        {
            if(depth > 0)
                depth--; // the `}` belongs to an expression or statement
            else
                break; // we found the closing `}` of the macro body
        }
        list_push(macro->replacing_tokens, next);
    }

    if(next->type != TOKEN_RBRACE)
        throw_error(ERR_SYNTAX_ERROR, next, "unexpected token `%s`, expect `}` to begin the macro body", next->value);

    Macro_T* found = find_macro(pp, macro->tok->value, macro->argc);
    if(found && found->tok)
        throw_error(ERR_REDEFINITION, macro->tok, "redefinition of macro `%s` with %d arguments\nfirst defined in `%s` at line %d", macro->tok->value, macro->argc, found->tok->source->path ? found->tok->source->path : found->tok->source->short_path, found->tok->line + 1);
    else if(found)
        throw_error(ERR_SYNTAX_ERROR, macro->tok, "redefinition of macro `%s` with %d arguments", macro->tok->value, macro->argc);

    return macro;
}

static Macro_T* find_macro(Preprocessor_T* pp, char* callee, u8 argc)
{
    for(size_t i = 0; i < pp->macros->size; i++)
    {
        Macro_T* mac = (Macro_T*) pp->macros->items[i];
        if(strcmp(mac->tok->value, callee) == 0 && mac->argc == argc)
            return mac;
    }
    return NULL;
}

static int find_macro_arg(Macro_T* mac, char* callee)
{
    for(uint8_t i = 0; i < mac->argc; i++)
    {
        Token_T* tok = mac->args[i];
        if(tok && strcmp(tok->value, callee) == 0)
            return i;
    }
    return -1;
}

//
// Macro call parsing
//

static void parse_macro_call(Preprocessor_T* pp, MacroCall_T* call, List_T* token_list, size_t* i)
{
    init_macro_call(pp, call, token_list->items[*i]);

    if((*i) + 1 >= token_list->size)
        return;

    Token_T* next = token_list->items[(*i) + 1];
    switch(next->type)
    {
        case TOKEN_LPAREN:
        case TOKEN_LBRACE:
        case TOKEN_LBRACKET:
            {
                (*i)++;

                if((*i) + 1 >= token_list->size)
                    throw_error(ERR_SYNTAX_ERROR, next, "unexpected end of macro body, expect `)`");

                TokenType_T opening_paren = next->type;
                TokenType_T closing_paren = next->type + 1;
                int64_t depth = 0, arg_start = (*i) + 1, arg_end = (*i) + 1;
                bool has_arg = false;

                while(((next = token_list->items[++(*i)])->type != closing_paren || depth != 0) && (*i) + 1 < token_list->size)
                {
                    if(next->type == TOKEN_LPAREN || next->type == TOKEN_LBRACKET || next->type == TOKEN_LBRACE) depth++;
                    if((next->type == TOKEN_RPAREN || next->type == TOKEN_RBRACKET || next->type == TOKEN_RBRACE) && depth > 0) depth--;
                    if(next->type == TOKEN_COMMA && depth == 0) 
                    {
                        if(call->argc >= __CSP_MAX_FN_NUM_ARGS)
                            throw_error(ERR_SYNTAX_ERROR, next, "too many arguments for macro call");

                        call->args[call->argc].start_idx = arg_start;
                        call->args[call->argc++].end_idx = arg_end - 1;
                        arg_start = arg_end = (*i) + 1;
                    }
                    else if(next->type == TOKEN_EOF)
                    {
                        throw_error(ERR_SYNTAX_ERROR, next, "unexpected end of file, expect closing `%c` after macro call",
                                    closing_paren == TOKEN_RPAREN ? ')' : closing_paren == TOKEN_RBRACKET ? ']' : '}');
                    }
                    else {
                        arg_end++;
                        has_arg = true;
                    }
                }

                if((*i) + 1 >= token_list->size && (next->type != closing_paren || depth != 0))
                    throw_error(ERR_SYNTAX_ERROR, next, "unexpected end of macro body, expect `)`");

                if(has_arg) 
                {
                    call->args[call->argc].start_idx = arg_start;
                    call->args[call->argc++].end_idx = arg_end - 1;
                }
            } break;
        default:
            break;
    }

    if(!(call->macro = find_macro(pp, call->tok->value, call->argc)))
    {
        throw_error(ERR_SYNTAX_ERROR, call->tok, "undefined macro `%s` with `%d` arguments", call->tok->value, call->argc);
    }
    else
        call->macro->used = true;
}

//
// Macro expansion
//

static void expand_macro_call(Preprocessor_T* pp, MacroCall_T call, List_T* src_list, List_T* dest_list)
{
    for(size_t i = 0; i < call.macro->replacing_tokens->size; i++)
    {
        Token_T* tok = call.macro->replacing_tokens->items[i];

        switch(tok->type) {
            case TOKEN_ID:
            {
                i32 arg_idx = find_macro_arg(call.macro, tok->value);
                if(arg_idx != -1)
                    for(size_t j = call.args[arg_idx].start_idx; j <= call.args[arg_idx].end_idx; j++)
                        list_push(dest_list, src_list->items[j]);
                else if(strcmp(tok->value, "__line__") == 0) 
                {
                    char linestr[128] = { '\0' };
                    sprintf(linestr, "%u", call.tok->line + 1);
                    list_push(dest_list, init_token(linestr, tok->line, tok->pos, TOKEN_INT, tok->source));
                }
                else if(strcmp(tok->value, "__file__") == 0)
                    list_push(dest_list, init_token((char*) call.tok->source->path, tok->line, tok->pos, TOKEN_STRING, tok->source));
                else if(strcmp(tok->value, "__func__") == 0)
                    list_push(dest_list, init_token("", tok->line, tok->pos, TOKEN_CURRENT_FN, tok->source));
                else
                    list_push(dest_list, tok);
            } break;

            case TOKEN_MACRO_CALL:
            {
                MacroCall_T macro_call;
                parse_macro_call(pp, &macro_call, call.macro->replacing_tokens, &i);
                expand_macro_call(pp, macro_call, call.macro->replacing_tokens, dest_list);
            } break;

            default:
                list_push(dest_list, tok);
                break;
        }
    }
}

//
// Main Preprocessor function
//

List_T* lex_and_preprocess_tokens(Lexer_T* lex, List_T* files, bool is_silent)
{
    Preprocessor_T pp;
    init_preprocessor(&pp, lex);

    pp.files = files;
    pp.is_silent = is_silent;

    /**************************************
    * Stage 0: lex the main file          *
    **************************************/

    Token_T* tok;
    for(tok = lexer_next_token(lex); tok->type != TOKEN_EOF; tok = lexer_next_token(lex))
        list_push(pp.tokens, tok);
    Token_T* eof = tok;

    /**************************************
    * Stage 1: lex and import all files   *
    **************************************/

    for(size_t i = 0; i < pp.tokens->size; i++)
    {
        tok = pp.tokens->items[i];
        if(tok->type == TOKEN_IMPORT)
            parse_import_def(&pp, pp.tokens, &i);
    }

    push_tok(&pp, eof);

    /***************************************
    * Stage 2: parse macro definitions     *
    ***************************************/

    List_T* token_stage_2 = init_list(sizeof(struct TOKEN_STRUCT*)); // init a new list for stage 2
    for(size_t i = 0; i < pp.tokens->size;)
    {
        tok = pp.tokens->items[i];
        if(tok->type == TOKEN_MACRO)
        {
            list_push(pp.macros, parse_macro_def(&pp, &i));
            continue;
        }
        list_push(token_stage_2, tok);
        i++;
    }

    /**************************************
    * Stage 3: expand all macro calls     *
    **************************************/

    List_T* token_stage_3 = init_list(sizeof(struct TOKEN_STRUCT*)); //q init a new list for stage 3
    for(size_t i = 0; i < token_stage_2->size; i++)
    {
        tok = token_stage_2->items[i];
        if(tok->type == TOKEN_MACRO_CALL)
        {
            MacroCall_T macro_call;
            parse_macro_call(&pp, &macro_call, token_stage_2, &i);
            expand_macro_call(&pp, macro_call, token_stage_2, token_stage_3);
            continue;
        }
        list_push(token_stage_3, tok);
    }


    free_list(pp.tokens);     // free from stage 0 & 1
    free_list(token_stage_2); // free from stage 1
    free_preprocessor(&pp);
    return token_stage_3;
}