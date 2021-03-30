#include "LLVMBackend.hpp"

#include <llvm/Bitcode/LLVMBitCodes.h>
#include <llvm/CodeGen/LinkAllCodegenComponents.h>

void compile(AST_T* ast)
{
    LOG_WARN("C++ Code called from C works! %s\n", ((AST_T*) ast->root->contents->items[0])->def->name);
}