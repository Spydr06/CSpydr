#include "llvm_generators.hpp"

#include <llvm/IR/LLVMContext.h>
#include <memory>

namespace CSpydrLLVMCodegen 
{
    ProgramGenerator::ProgramGenerator(ASTProg_T* ast_program)
        : ast(ast_program)
    {
    }

    ProgramGenerator::~ProgramGenerator()
    {
    }

    void ProgramGenerator::generate()
    {
        if(!codegen_data->silent)
            LOG_OK(COLOR_BOLD_BLUE "  Generating" COLOR_RESET " LLVM Code\n");
    }

    void ProgramGenerator::print_code()
    {

    }
}