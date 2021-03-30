#include "LLVMCompiler.hpp"
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <memory>
#include "../log.h"

namespace CSpydr
{
    LLVMCompiler::LLVMCompiler(std::string targetPath)
        : targetPath(targetPath)
    {
        llvmContext = std::make_unique<llvm::LLVMContext>();

        //TODO: add name detection from filepath
        llvmModule = std::make_unique<llvm::Module>("test", *llvmContext);
        llvmBuilder = std::make_unique<llvm::IRBuilder<>>(*llvmContext);
    }

    LLVMCompiler::~LLVMCompiler()
    {
    }

    void LLVMCompiler::compile()
    {
        LOG_OK(COLOR_BOLD_GREEN "Compiling " COLOR_RESET "to target \"%s\"...\n", targetPath.c_str());
    }
}