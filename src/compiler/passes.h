#ifndef CSPYDR_PASSES_H
#define CSPYDR_PASSES_H

#include "util.h"
#include "ast/ast.h"

i32 initialization_pass(ASTProg_T* ast);
i32 lexer_pass(ASTProg_T* ast);
i32 preprocessor_pass(ASTProg_T* ast);
i32 parser_pass(ASTProg_T* ast);
i32 validator_pass(ASTProg_T *ast);
i32 typechecker_pass(ASTProg_T *ast);
i32 optimizer_pass(ASTProg_T *ast);
i32 transpiler_pass(ASTProg_T *ast);
i32 asm_codegen_pass(ASTProg_T *ast);
i32 serializer_pass(ASTProg_T *ast);

#ifdef CSPYDR_USE_LLVM
    i32 llvm_codegen_pass(ASTProg_T* ast);
#endif

#endif