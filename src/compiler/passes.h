#ifndef CSPYDR_PASSES_H
#define CSPYDR_PASSES_H

#include "util.h"
#include "ast/ast.h"
#include "context.h"

#define PASS_FN_DECL(name) \
    i32 name##_pass(Context_T* context, ASTProg_T* ast)

PASS_FN_DECL(initialization);
PASS_FN_DECL(lexer);
PASS_FN_DECL(preprocessor);
PASS_FN_DECL(parser);
PASS_FN_DECL(validator);
PASS_FN_DECL(typechecker);
PASS_FN_DECL(optimizer);
PASS_FN_DECL(transpiler);
PASS_FN_DECL(asm_codegen);
PASS_FN_DECL(serializer);
PASS_FN_DECL(cleanup);

#ifdef CSPYDR_USE_LLVM
    PASS_FN_DECL(llvm_codegen);
#endif

#endif