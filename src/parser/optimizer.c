#include "optimizer.h"

#include <string.h>

optimizer_T* initOptimizer()
{
    optimizer_T* opt = malloc(sizeof(struct OPTIMIZER_STRUCT));

    opt->functions = initList(sizeof(struct AST_FUCTION_STRUCT*));
    opt->typedefs = initList(sizeof(struct AST_TYPEDEF_STRUCT*));
    opt->imports = initList(sizeof(struct AST_IMPORT_STRUCT*));
    opt->globals = initList(sizeof(struct AST_GLOBAL_STRUCT*));
    opt->locals = initList(sizeof(struct AST_LOCAL_STRUCT*));

    return opt;
}

void freeOptimizer(optimizer_T* opt)
{
    freeList(opt->functions);
    freeList(opt->typedefs);
    freeList(opt->imports);
    freeList(opt->globals);
    freeList(opt->locals);

    free(opt);
}

static ASTTypedef_T* findTypedef(optimizer_T* opt, char* tdef)
{
    for(int i = 0; i < opt->typedefs->size; i++)
    {
        ASTTypedef_T* tf = (ASTTypedef_T*) opt->typedefs->items[i];
        return tf;
    }
    return NULL;
}

void optimizeAST(optimizer_T* opt, ASTProgram_T* ast)
{
}