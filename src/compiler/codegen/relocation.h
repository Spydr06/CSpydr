#ifndef CSPYDR_RELOCATION_H
#define CSPYDR_RELOCATION_H

#include "ast/ast.h"
#include "util.h"

u8 escape_sequence(char c, const char* str, size_t* i);
void gen_relocation(ASTNode_T* node, size_t target_size, u8* buffer);

#endif
