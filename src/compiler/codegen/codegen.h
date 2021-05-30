#ifndef CSPYDR_CODEGEN_H
#define CSPYDR_CODEGEN_H

#include "../ast/ast.h"
#include "../error/errorHandler.h"
#include "../io/log.h"

#include <stdio.h>

extern const char *argreg8[];
extern const char *argreg16[];
extern const char *argreg32[];
extern const char *argreg64[];

typedef struct CODE_GENERATOR_STRUCT {
    ASTFunction_T* current_func;
    int depth;
    int count;

    FILE* output_file;
    errorHandler_T* eh;
} codeGenerator_T;

codeGenerator_T* init_generator(errorHandler_T* eh);
void free_generator(codeGenerator_T* cg);
void generate_asm(codeGenerator_T* cg, ASTProgram_T* prog, const char* target);

#endif