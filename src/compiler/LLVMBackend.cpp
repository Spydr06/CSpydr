#include "LLVMBackend.hpp"

#include "LLVMIncludes.hpp"
#include "LLVMCompiler.hpp"

#include "utils/StringUtils.hpp"

void compile(AST_T* ast, const char* targetPath, const char* sourcePath)
{

    CSpydr::LLVMCompiler compiler(targetPath, getFilenameFromPath(sourcePath));
    compiler.compile();

}