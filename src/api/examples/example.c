#include <cspydr.h>

int main() 
{
    Compiler_T* compiler = init_compiler();

    // do something with compiler

    free_compiler(compiler);
}