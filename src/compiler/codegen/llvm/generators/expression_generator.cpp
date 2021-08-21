#include "llvm_generators.hpp"
#include <llvm/IR/Value.h>

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

