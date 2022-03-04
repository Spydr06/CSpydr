#ifndef CSPYDR_CODEGEN_UTILS_H
#define CSPYDR_CODEGEN_UTILS_H

#include <stdint.h>
#include "../ast/ast.h"

#define MAX(a, b) ((a) < (b) ? (b) : (a))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

u64 hash_64(const char* key);
char* gen_identifier(ASTIdentifier_T* id, const char* prefix, bool prefix_at_start);
bool is_integer(ASTType_T *ty);
bool is_flonum(ASTType_T *ty);
bool is_numeric(ASTType_T *ty);
bool is_unsigned(ASTType_T* ty);
char* find_gcc_libpath(void);
char* find_libpath(void);
bool unsigned_type(ASTType_T* ty);
bool vla_type(ASTType_T* ty);
bool is_variadic(ASTObj_T* fn);
ASTType_T* unpack(ASTType_T* ty);
bool should_emit(ASTObj_T* obj);

#endif