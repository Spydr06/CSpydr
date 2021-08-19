#include "llvm_generators.hpp"

namespace CSpydrLLVMCodegen 
{
    ExpressionGenerator::ExpressionGenerator(ASTNode_T* ast_expression) 
        : ast(ast_Expression)
    {
    }

    ExpressionGenerator::~ExpressionGenerator()
    {
    }

    llvm::Value ExpressionGenerator::generate() 
    {
    }
}

