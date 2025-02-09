#ifndef CSPYDR_RELOCATION_H
#define CSPYDR_RELOCATION_H

#include "ast/ast.h"
#include "util.h"
#include "asm_codegen.h"

u8 escape_sequence(char c, const char* str, size_t* i);
size_t gen_relocation(ASMCodegenData_T* cg, ASTNode_T* node, size_t target_size);

#endif
