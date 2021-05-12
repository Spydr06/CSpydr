#pragma once

#include "llvm_includes.hpp"
#include "../../ast/ast.h"
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Function.h>
#include <memory>

namespace CSpydr
{
    class LLVMGenerator
    {
    public:
        LLVMGenerator(std::string module);
        ~LLVMGenerator();

        enum status {
            STATUS_OK,
            STATUS_ERROR,
            STATUS_WARNINGS
        };

        status generate(ASTProgram_T* ast);
        void generateFile(ASTFile_T *ast);
    private:
        llvm::GlobalVariable* generateGlobalVar(ASTGlobal_T* global);
        llvm::Function* createFunction(ASTFunction_T* func);
        llvm::Function* generateFunction(ASTFunction_T* func);
        llvm::BasicBlock* generateCompound(ASTCompound_T* com, llvm::Function* funcRef, std::string name = "entry");
        llvm::Type* generateType(ASTType_T* type);

        std::string moduleName;

        std::unique_ptr<llvm::LLVMContext> LLVMContext;
        std::unique_ptr<llvm::Module> llvmModule;
        std::unique_ptr<llvm::IRBuilder<>> llvmBuilder;
        std::map<std::string, llvm::Value*> namedValues;
    };
};