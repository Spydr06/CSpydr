#include <cspydr.h>
#include <stdio.h>

int main() 
{
    CSpydrCompiler_T* compiler = csp_init_compiler();

    // do something with compiler
    puts(csp_status_str(csp_get_status(compiler)));

    csp_free_compiler(compiler);
    return 0;
}