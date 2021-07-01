#ifndef CSPYDR_PREPROCESSOR_H
#define CSPYDR_PREPROCESSOR_H

#include "../list.h"
#include "../ast/ast.h"
#include "../error/error.h"

typedef struct OPTIMIZER_STRUCT
{
    List_T* vars;
    List_T* fns;
    List_T* tdefs;

    int num_errors_found;
} Optimizer_T;

Optimizer_T* init_optimizer(void);
void         free_optimizer(Optimizer_T* pp);

void optimize(ASTProg_T* ast);

#endif