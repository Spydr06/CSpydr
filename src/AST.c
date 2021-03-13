#include "AST.h"
#include <stdlib.h>
#include <string.h>

AST_T* initAST(int type)
{
    AST_T* ast = calloc(1, sizeof(struct AST_STRUCT));
    ast->type = type;
    
    if(type == AST_COMPOUND || type == AST_ROOT || type == AST_FUNCTION) {
        ast->children = initList(sizeof(struct AST_STRUCT*));
    }

    return ast;
}

int typenameToInt(const char *name) 
{
    int t = 0;
    size_t len = strlen(name);

    for(unsigned int i = 0; i < len; i++) {
        t += name[i];
    }

    return t;
}