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
    ASTFunction_T* current_fn;
    int depth;
    int count;

    FILE* output_file;
    errorHandler_T* eh;
} CodeGen_T;

CodeGen_T* init_generator(errorHandler_T* eh);
void free_generator(CodeGen_T* cg);
void generate_asm(CodeGen_T* cg, ASTProgram_T* prog, const char* target);

void assign_lvar_offsets(ASTFunction_T* fn);
void assign_alloca_size(ASTFunction_T* fn);

#endif