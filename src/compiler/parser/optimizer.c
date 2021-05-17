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

static void optimizeType(optimizer_T* opt, ASTType_T* type)
{
    if(type->type == AST_TYPEDEF)
    {
        if(!findTypedef(opt, type->callee))
        {
            const char* template = "undefined type `%s`";
        char* msg = calloc(strlen(template) + strlen(type->callee) + 1, sizeof(char));
        sprintf(msg, template, type->callee);
        throwUndefinitionError(opt->eh, msg, type->line, type->pos);
        }
    }
}

static void optimizeGlobal(optimizer_T* opt, ASTGlobal_T* gl)
{
   optimizeType(opt, gl->type);
}

static void optimizeLocal(optimizer_T* opt, ASTLocal_T* loc)
{
    optimizeType(opt, loc->dataType);
}

static void optimizeStmt(optimizer_T* opt, ASTStmt_T* stmt)
{
    switch(stmt->type)
    {
        case STMT_LET:
            optimizeLocal(opt, (ASTLocal_T*) stmt->stmt);
            break;
    }
}

static void optimizeCompund(optimizer_T* opt, ASTCompound_T* com)
{
    for(int i = 0; i < com->stmts->size; i++)
    {
        optimizeStmt(opt, (ASTStmt_T*)com->stmts->items[i]);    
    }
}

static void optimizeFunction(optimizer_T* opt, ASTFunction_T* func)
{
    optimizeType(opt, func->returnType);
    optimizeCompund(opt, func->body);
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

    for(int i = 0; i < file->functions->size; i++)
        optimizeFunction(opt, (ASTFunction_T*) file->functions->items[i]);
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