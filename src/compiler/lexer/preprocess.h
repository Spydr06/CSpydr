#ifndef CSPYDR_PREPROCESSOR_H
#define CSPYDR_PREPROCESSOR_H

#include "../list.h"
#include "lexer.h"

typedef struct PREPROCESSOR_STRUCT
{
    Lexer_T* lex;

    List_T* output_tokens;
    List_T* macros;
} Preprocessor_T;

List_T* lex_and_preprocess_tokens(Lexer_T* lex);

#endif