#pragma once

#include "../llvm_cpp_layer.hpp"

#include "../../../ast/ast.h"
#include "../../../io/log.h"

#include "llvm/IR/Function.h"
#include "llvm/IR/Type.h"
#include <llvm/IR/GlobalValue.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Value.h>
#include <memory>

namespace CSpydrLLVMCodegen 
{
    class LLVMGenerator // all Generators are of this type
    {
    };

    class ProgramGenerator : LLVMGenerator 
    {
    public:
        ProgramGenerator(ASTProg_T* ast_program);
        ~ProgramGenerator();

        void generate();
        void print_code();
    private:
        ASTProg_T* ast;
    };

    class FunctionGenerator : LLVMGenerator
    {
    public:
        FunctionGenerator(ASTObj_T* ast_function);
        ~FunctionGenerator();
        llvm::Function generate();

    private:
        ASTObj_T* ast;
    };

    class GlobalGenerator : LLVMGenerator 
    {
    public:
        GlobalGenerator(ASTObj_T* ast_global);
        ~GlobalGenerator();
        llvm::GlobalValue generate();

    private:
        ASTObj_T* ast;
    };

    class TypeGenerator : LLVMGenerator
    {
    public:
        TypeGenerator(ASTType_T* ast_type);
        ~TypeGenerator();
        llvm::Type generate();
    private:
        ASTType_T* ast;
    };

    class ExpressionGenerator : LLVMGenerator
    {
    public:
        ExpressionGenerator(ASTNode_T* ast_expression);
        ~ExpressionGenerator();

        llvm::Value generate();
    private:
        ASTNode_T* ast;
    };
}