#include "llvm_generators.hpp"

namespace CSpydrLLVMCodegen 
{
    ExpressionGenerator::ExpressionGenerator(ASTNode_T* ast_expression) 
        : ast(ast_expression)
    {
    }

    ExpressionGenerator::~ExpressionGenerator()
    {
    }

    llvm::Value ExpressionGenerator::generate() 
    {
    }
}

