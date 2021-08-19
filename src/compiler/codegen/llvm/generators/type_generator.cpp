#include "llvm_generators.hpp"

namespace CSpydrLLVMCodegen 
{
    TypeGenerator::TypeGenerator(ASTType_T* ast_type)
        : ast(ast_type)
    {
    }

    TypeGenerator::~TypeGenerator()
    {
    }

    llvm::Type TypeGenerator::generate()
    {
    }
}