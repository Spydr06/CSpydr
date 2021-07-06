#ifndef CSPYDR_PREPROCESSOR_H
#define CSPYDR_PREPROCESSOR_H

#include "../list.h"
#include "lexer.h"

typedef struct PREPROCESSOR_STRUCT
{
    Lexer_T* lex;
    List_T* files;

    List_T* tokens;
    List_T* macros;
    List_T* imports;
} Preprocessor_T;

List_T* lex_and_preprocess_tokens(Lexer_T* lex, List_T* files);

#endif