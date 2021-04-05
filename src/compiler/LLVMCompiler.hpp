#pragma once
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/GlobalValue.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Verifier.h>
#include <memory>
#include "LLVMIncludes.hpp"
#include "../core/parser/AST.h"

namespace CSpydr
{

    class LLVMCompiler 
    {
    public:
        LLVMCompiler(std::string targetPath, std::string sourcePath, bool emitDebugInfo = false);
        ~LLVMCompiler();

        void compile(ASTRoot_T* ast);

        llvm::Function* generateFunction(ASTFunction_T* ASTFunc);
        llvm::Function* createFunction(ASTFunction_T* ASTFunc);

        llvm::BasicBlock* generateCompound(ASTCompound_T* ASTCompound, llvm::Function* functionRef, std::string name = "entry");

        llvm::GlobalVariable* generateGlobalVar(ASTGlobal_T* ASTGlobal);
        llvm::AllocaInst* generateLocalVar(ASTLocal_T* ASTLocal, llvm::BasicBlock* surroundingBlock);
        llvm::Value* generateFunctionCall(ASTExprFnCall_T* ASTCall);
        llvm::Value* generateVariableCall(ASTExprVarCall_T* ASSTCall);

        llvm::Type* generateLLVMType(ASTDataType_T* ast);
    private:
        bool emitDebugInfo;

        std::string targetPath;
        std::string moduleName;

        std::unique_ptr<llvm::LLVMContext> llvmContext;
        std::unique_ptr<llvm::Module> llvmModule;
        std::unique_ptr<llvm::IRBuilder<>> llvmBuilder;
        std::map<std::string, llvm::Value*> namedValues;
    };

}