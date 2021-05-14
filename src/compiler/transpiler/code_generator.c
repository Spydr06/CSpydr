#include "transpiler.h"
#include <string.h>

#define ADD_CODE(str, sec) sec = realloc(sec, (sizeof(sec) + sizeof(str) + 1) * sizeof(char)); \
                           strcat(sec, str)
#define ADD_INCL(str, tp) ADD_CODE(str, tp->inclSection);
#define ADD_TYPE(str, tp) ADD_CODE(str, tp->typeSection);
#define ADD_DEF(str, tp) ADD_CODE(str, tp->defSection);
#define ADD_IMPL(str, tp) ADD_CODE(str, tp->implSection);

void generateCCode(transpiler_T* tp, ASTProgram_T* ast)
{
    ADD_INCL("#include <stdio.h>\n#include <stdlib.h>\n#include <string.h>", tp);

    for(int i = 0; i < ast->files->size; i++)
    {

    }
}