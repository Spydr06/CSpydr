#include "llvm_backend.hpp"
#include "llvm_generator.hpp"

#include "utils/stringutils.hpp"

#include <iostream>

void generateLLVM(ASTProgram_T *ast, const char *targetPath, const char *sourcePath)
{
    LOG_OK(COLOR_BOLD_GREEN "Generating" COLOR_RESET " llvm code\n");

    CSpydr::LLVMGenerator* generator = new CSpydr::LLVMGenerator(getFilenameFromPath(std::string(sourcePath)));

    generator->generate(ast);

    delete generator;
}