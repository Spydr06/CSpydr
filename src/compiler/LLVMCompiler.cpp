#include "LLVMCompiler.hpp"
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <memory>
#include "../log.h"

namespace CSpydr
{
    LLVMCompiler::LLVMCompiler(std::string targetPath, std::string moduleName)
        : targetPath(targetPath), moduleName(moduleName)
    {
        llvmContext = std::make_unique<llvm::LLVMContext>();
        
        llvmModule = std::make_unique<llvm::Module>(moduleName, *llvmContext);
        llvmBuilder = std::make_unique<llvm::IRBuilder<>>(*llvmContext);
    }

    LLVMCompiler::~LLVMCompiler()
    {
    }

    void LLVMCompiler::compile()
    {
        LOG_OK(COLOR_BOLD_GREEN "Compiling" COLOR_RESET " module \"%s\" to file \"%s\"\n", moduleName.c_str(), targetPath.c_str());
    }
}