#ifndef CSPYDR_PREPROCESSOR_H
#define CSPYDR_PREPROCESSOR_H

#include "../list.h"
#include "../lexer/lexer.h"

#include <stdint.h>

typedef struct MACRO_STRUCT
{
    Token_T* tok;
    List_T* replacing_tokens;

    u8 argc;
    Token_T* args[__CSP_MAX_FN_NUM_ARGS];
    bool used : 1;
} __attribute__((packed)) Macro_T;

i32 preprocessor_pass(Context_T* context, ASTProg_T* ast);
Macro_T* init_macro(Token_T* tok);

#endif