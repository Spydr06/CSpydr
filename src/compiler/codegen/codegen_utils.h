#ifndef CSPYDR_CODEGEN_UTILS_H
#define CSPYDR_CODEGEN_UTILS_H

#include <stdint.h>
#include "../ast/ast.h"

#define MAX(a, b) (a > b ? a : b)
#define MIN(a, b) (a < b ? a : b)

uint64_t hash_64(const char* key);
char* gen_identifier(ASTIdentifier_T* id);
bool is_integer(ASTType_T *ty);
bool is_flonum(ASTType_T *ty);
bool is_numeric(ASTType_T *ty);
bool is_unsigned(ASTType_T* ty);
int align_to(int n, int align);

#endif