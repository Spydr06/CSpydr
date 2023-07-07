#ifndef CSPYDR_AST_JSON_H
#define CSPYDR_AST_JSON_H

#include "ast.h"

i32 serializer_pass(Context_T* context, ASTProg_T* ast);

void ast_to_json(ASTProg_T* ast, const char* file, bool print_json);
void ast_from_json(ASTProg_T* ast, const char* file);

#endif