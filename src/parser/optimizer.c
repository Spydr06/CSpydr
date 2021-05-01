#include "optimizer.h"
#include "../io/log.h"

#include <string.h>
#include <stdio.h>

optimizer_T* initOptimizer(errorHandler_T* eh)
{
    optimizer_T* opt = malloc(sizeof(struct OPTIMIZER_STRUCT));

    opt->functions = initList(sizeof(struct AST_FUCTION_STRUCT*));
    opt->typedefs = initList(sizeof(struct AST_TYPEDEF_STRUCT*));
    opt->globals = initList(sizeof(struct AST_GLOBAL_STRUCT*));
    opt->locals = initList(sizeof(struct AST_LOCAL_STRUCT*));

    opt->eh = eh;
    opt->errors = 0;

    return opt;
}

void freeOptimizer(optimizer_T* opt)
{
    freeList(opt->functions);
    freeList(opt->typedefs);
    freeList(opt->globals);
    freeList(opt->locals);

    free(opt);
}

static ASTTypedef_T* findTypedef(optimizer_T* opt, char* tdef)
{
    for(int i = 0; i < opt->typedefs->size; i++)
    {
        ASTTypedef_T* tf = (ASTTypedef_T*) opt->typedefs->items[i];
        if(strcmp(tf->name, tdef) == 0)
            return tf;
    }
    return NULL;
}

static ASTGlobal_T* findGlobal(optimizer_T* opt, char* callee)
{
    for(int i = 0; i < opt->globals->size; i++)
    {
        ASTGlobal_T* gl = (ASTGlobal_T*) opt->globals->items[i];
        if(strcmp(gl->name, callee) == 0)
            return gl;
    }
    return NULL;
}

static ASTFunction_T* findFunction(optimizer_T* opt, char* callee)
{
    for(int i = 0; i < opt->functions->size; i++)
    {
        ASTFunction_T* fn = (ASTFunction_T*) opt->functions->items[i];
        if(strcmp(fn->name, callee) == 0)
            return fn;
    }
    return NULL;
}

static void registerTypedef(optimizer_T* opt, ASTTypedef_T* tdef)
{
    ASTTypedef_T* ast = findTypedef(opt, tdef->name);
    if(ast != NULL)
    {
        opt->errors++;

        // TODO: include the file path
        const char* template = "redefinition of type `%s`; first defined:" COLOR_BOLD_WHITE " %ld:%ld" COLOR_RESET;
        char* msg = calloc(strlen(template) + strlen(tdef->name) + 1, sizeof(char));
        sprintf(msg, template, tdef->name, ast->line + 1, ast->pos + 1);
        throwRedefinitionError(opt->eh, msg, tdef->line, tdef->pos);
        return;
    }        

    listPush(opt->typedefs, tdef);
}

static void registerGlobal(optimizer_T* opt, ASTGlobal_T* gl)
{
    ASTGlobal_T* ast = findGlobal(opt, gl->name);
    if(ast != NULL)
    {
        opt->errors++;

        // TODO: include the file path
        const char* template = "redefinition of global variable `%s`; first defined:" COLOR_BOLD_WHITE " %ld:%ld" COLOR_RESET;
        char* msg = calloc(strlen(template) + strlen(gl->name) + 1, sizeof(char));
        sprintf(msg, template, gl->name, ast->line + 1, ast->pos + 1);
        throwRedefinitionError(opt->eh, msg, gl->line, gl->pos);
        return;
    }

    listPush(opt->globals, gl);
}

static void registerFunction(optimizer_T* opt, ASTFunction_T* fn)
{
    ASTFunction_T* ast = findFunction(opt, fn->name);
    if(ast != NULL)
    {
        opt->errors++;

        // TODO: include the file path
        const char* template = "redefinition of function `%s`; first defined:" COLOR_BOLD_WHITE " %ld:%ld" COLOR_RESET;
        char* msg = calloc(strlen(template) + strlen(fn->name) + 1, sizeof(char));
        sprintf(msg, template, fn->name, ast->line + 1, ast->pos + 1);
        throwRedefinitionError(opt->eh, msg, fn->line, fn->pos);
        return;
    }

    listPush(opt->functions, fn);
}

static void optimizeGlobal(optimizer_T* opt, ASTGlobal_T* gl)
{
    if(gl->type->type == AST_TYPEDEF)   // resolve typedefs
    {
        ASTTypedef_T* tdef = findTypedef(opt, gl->type->callee);
        if(!tdef) 
        {
            opt->errors++;

            const char* template = "undefined type `%s`";
            char* msg = calloc(strlen(template) + strlen(gl->type->callee) + 1, sizeof(char));
            sprintf(msg, template, gl->type->callee);
            throwUndefinitionError(opt->eh, msg, gl->line, gl->pos);
        }

        freeASTType(gl->type);
        gl->type = tdef->dataType;
        gl->typeHasToBeFreed = false;
    }
}

static void optimizeASTFile(optimizer_T* opt, ASTFile_T* file)
{
    // first, register all contents of the file. This needs to happen first, because the Order of the components does not matter in Spydr
    for(int i = 0; i < file->types->size; i++)
        registerTypedef(opt, (ASTTypedef_T*) file->types->items[i]);

    for(int i = 0; i < file->globals->size; i++)
        registerGlobal(opt, (ASTGlobal_T*) file->globals->items[i]);

    for(int i = 0; i < file->functions->size; i++)
        registerFunction(opt, (ASTFunction_T*) file->functions->items[i]);

    // Optimize everything
    for(int i = 0; i < file->globals->size; i++)
        optimizeGlobal(opt, (ASTGlobal_T*) file->globals->items[i]);
}

void optimizeAST(optimizer_T* opt, ASTProgram_T* ast)
{
    for(int i = 0; i < ast->files->size; i++)
    {
        ASTFile_T* fileAST = (ASTFile_T*) ast->files->items[i];

        optimizeASTFile(opt, fileAST);
    }

    if(opt->errors > 0)
    {
        if(opt->errors == 1)
            LOG_ERROR("Encountered 1 error.\n");
        else 
            LOG_ERROR_F("Encountered %d errors.\n", opt->errors);

        free(opt);
        freeASTProgram(ast);

        exit(1);
    }
}