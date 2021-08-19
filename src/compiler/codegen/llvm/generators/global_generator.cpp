#include "llvm_generators.hpp"

namespace CSpydrLLVMCodegen 
{
    GlobalGenerator::GlobalGenerator(ASTObj_T* ast_global)
        : ast(ast_global)
    {
    }

    GlobalGenerator::~GlobalGenerator()
    {
    }

    llvm::GlobalValue GlobalGenerator::generate()
    {
    }
}