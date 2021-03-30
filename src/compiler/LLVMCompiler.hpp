#pragma once

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>
#include <memory>
#include "LLVMIncludes.hpp"

namespace CSpydr
{

    class LLVMCompiler 
    {
    public:
        LLVMCompiler(std::string targetPath);
        ~LLVMCompiler();

        void compile();

    private:
        std::string targetPath;

        std::unique_ptr<llvm::LLVMContext> llvmContext;
        std::unique_ptr<llvm::Module> llvmModule;
        std::unique_ptr<llvm::IRBuilder<>> llvmBuilder;
        std::map<std::string, llvm::Value *> namedValues;
    };

}