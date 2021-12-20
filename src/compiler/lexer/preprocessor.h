#ifndef CSPYDR_PREPROCESSOR_H
#define CSPYDR_PREPROCESSOR_H

#include "../list.h"
#include "lexer.h"

#include <stdint.h>

typedef struct MACRO_STRUCT
{
    Token_T* tok;
    List_T* replacing_tokens;

    uint8_t argc;
    Token_T* args[__CSP_MAX_FN_NUM_ARGS];
    bool used : 1;
} __attribute__((packed)) Macro_T;

List_T* lex_and_preprocess_tokens(Lexer_T* lex, List_T* files, bool is_silent);
Macro_T* init_macro(Token_T* tok);

#endif