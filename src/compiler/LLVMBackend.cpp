#include "LLVMBackend.hpp"

#include "LLVMIncludes.hpp"
#include "LLVMCompiler.hpp"

void compile(AST_T* ast, const char* targetPath)
{

    CSpydr::LLVMCompiler compiler(targetPath);
    compiler.compile();

}