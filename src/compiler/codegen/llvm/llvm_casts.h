#ifndef CSPYDR_LLVM_CAST_H
#define CSPYDR_LLVM_CAST_H

#include "llvm_codegen.h"
#include "../../ast/types.h"
#include <llvm-c/Types.h>

typedef LLVMValueRef (*LLVMCastFn)(LLVMCodegenData_T* cg, ASTNode_T* cast);

extern LLVMCastFn llvm_cast_map[NUM_TYPES][NUM_TYPES];

#endif