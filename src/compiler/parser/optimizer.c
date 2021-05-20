#include "optimizer.h"
#include "../io/log.h"

#include <string.h>
#include <stdio.h>

static void optimizeExpression(optimizer_T* opt, ASTExpr_T* expr);

optimizer_T* initOptimizer(errorHandler_T* eh)
{
    optimizer_T* opt = malloc(sizeof(struct OPTIMIZER_STRUCT));

    opt->functions = initList(sizeof(struct AST_FUCTION_STRUCT*));
    opt->typedefs = initList(sizeof(struct AST_TYPEDEF_STRUCT*));
    opt->globals = initList(sizeof(struct AST_GLOBAL_STRUCT*));
    opt->locals = initList(sizeof(struct AST_LOCAL_STRUCT*));
    opt->args = initList(sizeof(struct AST_ARGUMENT_STRUCT*));

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
    freeList(opt->args);

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

static ASTLocal_T* findLocal(optimizer_T* opt, char* callee)
{
    for(int i = 0; i < opt->locals->size; i++)
    {
        ASTLocal_T* loc = (ASTLocal_T*) opt->locals->items[i];
        if(strcmp(loc->name, callee) == 0)
            return loc;
    }
    return NULL;
}

static ASTArgument_T* findArgument(optimizer_T* opt, char* callee)
{
    for(int i = 0; i < opt->args->size; i++)
    {
        ASTArgument_T* arg = (ASTArgument_T*) opt->args->items[i];
        if(strcmp(arg->name, callee) == 0)
            return arg;
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
    if(ast)
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

static void registerLocal(optimizer_T* opt, ASTLocal_T* loc)
{
    ASTLocal_T* ast = findLocal(opt, loc->name);
    if(ast)
    {
        opt->errors++;

        // TODO: include the file path
        const char* template = "redefinition of local variable `%s`; first defined:" COLOR_BOLD_WHITE " %ld:%ld" COLOR_RESET;
        char* msg = calloc(strlen(template) + strlen(loc->name) + 1, sizeof(char));
        sprintf(msg, template, loc->name, ast->line + 1, ast->pos + 1);
        throwRedefinitionError(opt->eh, msg, loc->line, loc->pos);
        return;
    }

    listPush(opt->locals, loc);
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

static int typeIsPtr(optimizer_T* opt, ASTType_T* type)
{
    int isPtr = type->type == AST_POINTER; //Temporary
    if(type->type == AST_TYPEDEF)
    {
        ASTTypedef_T* tdef = findTypedef(opt, type->callee);
        if(tdef)
        {
            isPtr = tdef->dataType->type == AST_POINTER;
        }
        else
        {
            throwUndefinitionError(opt->eh, "Undefined type", type->line, type->pos);
        }
    }
    return isPtr;
}

static void optimizeIdentifier(optimizer_T* opt, ASTExpr_T* expr)
{
    ASTIdentifier_T* id = (ASTIdentifier_T*) expr->expr;
    ASTLocal_T* loc = findLocal(opt, id->callee);
    if(loc)
    {
        id->isPtr = typeIsPtr(opt, loc->dataType);
        return; 
    }

    ASTGlobal_T* gl = findGlobal(opt, id->callee);
    if(gl)
    {
        id->isPtr = typeIsPtr(opt, gl->type);
        return;
    }

    ASTArgument_T* arg = findArgument(opt, id->callee);
    if(arg)
    {
        id->isPtr = typeIsPtr(opt, arg->dataType);
        return;
    }
}

static void optimizeInfixExpr(optimizer_T* opt, ASTExpr_T* expr)
{
    ASTInfix_T* ifx = (ASTInfix_T*) expr->expr;

    optimizeExpression(opt, ifx->left);
    optimizeExpression(opt, ifx->right);   
}

static void optimizePrefixExpr(optimizer_T* opt, ASTExpr_T* expr)
{
    ASTPrefix_T* pfx = (ASTPrefix_T*) expr->expr;

    optimizeExpression(opt, pfx->right);   
}

static void optimizePostfixExpr(optimizer_T* opt, ASTExpr_T* expr)
{
    ASTPostfix_T* pfx = (ASTPostfix_T*) expr->expr;

    optimizeExpression(opt, pfx->left);   
}

static void optimizeCallExpr(optimizer_T* opt, ASTExpr_T* expr)
{
    ASTCall_T* call = (ASTCall_T*) expr->expr;

    for(int i = 0; i < call->args->size; i++)
    {
        optimizeExpression(opt, (ASTExpr_T*) call->args->items[i]);
    }
}

static void optimizeIndexExpr(optimizer_T* opt, ASTExpr_T* expr)
{
    ASTIndex_T* idx = (ASTIndex_T*) expr->expr;

    optimizeExpression(opt, idx->idx);
    optimizeExpression(opt, idx->value); 
}

static void optimizeExpression(optimizer_T* opt, ASTExpr_T* expr)
{
    switch(expr->type)
    {
        case EXPR_IDENTIFIER:
            optimizeIdentifier(opt,expr);
            break;
        case EXPR_INFIX:
            optimizeInfixExpr(opt, expr);
            break;
        case EXPR_PREFIX:
            optimizePrefixExpr(opt, expr);    
            break;
        case EXPR_POSTFIX:
            optimizePostfixExpr(opt, expr);
            break;
        case EXPR_CALL:
            optimizeCallExpr(opt, expr);
            break;
        case EXPR_INDEX:
            optimizeIndexExpr(opt, expr);
            break;
    }
}

static void optimizeGlobal(optimizer_T* opt, ASTGlobal_T* gl)
{
    optimizeType(opt, gl->type);
   
    if(gl->value)
        optimizeExpression(opt, gl->value);
}

static void optimizeLocal(optimizer_T* opt, ASTLocal_T* loc)
{
    optimizeType(opt, loc->dataType);

    if(loc->value)
        optimizeExpression(opt, loc->value);
}

static void optimizeCompund(optimizer_T* opt, ASTCompound_T* com)
{
    unsigned int localCount = 0;

    for(int i = 0; i < com->stmts->size; i++)
    {
        ASTStmt_T* stmt = (ASTStmt_T*) com->stmts->items[i];
        switch(stmt->type)
        {
        case STMT_LET:
            registerLocal(opt, (ASTLocal_T*) stmt->stmt);
            localCount++;
            optimizeLocal(opt, (ASTLocal_T*) stmt->stmt);

            break;
        case STMT_EXPRESSION:
            optimizeExpression(opt, ((ASTExprStmt_T*) stmt->stmt)->expr);
            break;
        case STMT_LOOP:
            optimizeExpression(opt, ((ASTLoop_T*) stmt->stmt)->condition);
            optimizeCompund(opt, ((ASTLoop_T*) stmt->stmt)->body);
            break;
        case STMT_IF:
            optimizeExpression(opt, ((ASTIf_T*) stmt->stmt)->condition);
            optimizeCompund(opt, ((ASTIf_T*) stmt->stmt)->ifBody);
            if(((ASTIf_T*) stmt->stmt)->elseBody)
                optimizeCompund(opt, ((ASTIf_T*) stmt->stmt)->elseBody);
            break;
        case STMT_RETURN:
            optimizeExpression(opt, ((ASTReturn_T*) stmt->stmt)->value);
        }
    }

    for(; localCount > 0; localCount--)
    {
        opt->locals->size--;
    }
}

static void optimizeFunction(optimizer_T* opt, ASTFunction_T* func)
{
    optimizeType(opt, func->returnType);

    unsigned int argc = 0;
    for(int i = 0; i < func->args->size; i++)
    {
        argc++;
        listPush(opt->args, (ASTArgument_T*) func->args->items[i]);
    }

    optimizeCompund(opt, func->body);

    opt->args->size -= argc;
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