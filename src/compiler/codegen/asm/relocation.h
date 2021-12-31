#ifndef CSPYDR_RELOCATION_H
#define CSPYDR_RELOCATION_H

#include "../../ast/ast.h"

typedef struct RELOCATION_STRUCT
{
    i32 offset;
    char** labe;
    i64 addend;
} Relocation_T;

void gen_relocation(Relocation_T* rel, ASTObj_T* obj);

#endif