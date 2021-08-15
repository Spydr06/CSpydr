#pragma once

#include "llvm_c_bindings.h"
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>
#include <memory>

namespace CSpydrLLVMCodegen 
{
    typedef struct LLVM_CODEGEN_DATA_STRUCT 
    {
        std::unique_ptr<llvm::LLVMContext> context;
        std::unique_ptr<llvm::Module> module;
        std::unique_ptr<llvm::IRBuilder<>> builder;

        bool silent;

        LLVM_CODEGEN_DATA_STRUCT() 
        {
            silent = false;
            context = std::make_unique<llvm::LLVMContext>();
            module = std::make_unique<llvm::Module>("Module", *context);
            builder = std::unique_ptr<llvm::IRBuilder<>>(new llvm::IRBuilder<>(*context));
        }
    } LLVMCodegenData_T; // all the important data for LLVM code generation

    extern std::shared_ptr<LLVMCodegenData_T> codegen_data;
}