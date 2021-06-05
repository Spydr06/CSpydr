#ifndef CSPYDR_PREPROCESSOR_H
#define CSPYDR_PREPROCESSOR_H

#include "../list.h"
#include "../ast/ast.h"
#include "../error/error.h"

typedef struct PREPROCESSOR_STRUCT
{
    List_T* vars;
    List_T* fns;
    List_T* tdefs;

    int num_errors_found;
} Preprocessor_T;

Preprocessor_T* init_preprocessor(void);
void            free_preprocessor(Preprocessor_T* pp);

void preprocess(ASTProg_T* ast);

#endif