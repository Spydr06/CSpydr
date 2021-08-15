#include "llvm_generators.hpp"

namespace CSpydrLLVMCodegen 
{
    FunctionGenerator::FunctionGenerator(ASTObj_T* ast_function)
        : ast(ast_function)
    {
    }

    FunctionGenerator::~FunctionGenerator()
    {
    }

    llvm::Function FunctionGenerator::generate()
    {
    }
}