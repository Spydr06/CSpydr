#include "preprocessor.h"
#include "../io/log.h"
#include "../ast/types.h"

#include <string.h>
#include <stdio.h>

static void optimize_expression(Preprocessor_T* opt, ASTExpr_T* expr);

Preprocessor_T* init_preprocessor(ErrorHandler_T* eh)
{
    Preprocessor_T* pre = malloc(sizeof(struct PREPROCESSOR_STRUCT));

    pre->functions = init_list(sizeof(struct AST_FUCTION_STRUCT*));
    pre->typedefs = init_list(sizeof(struct AST_TYPEDEF_STRUCT*));
    pre->globals = init_list(sizeof(struct AST_GLOBAL_STRUCT*));
    pre->locals = init_list(sizeof(struct AST_LOCAL_STRUCT*));
    pre->args = init_list(sizeof(struct AST_ARGUMENT_STRUCT*));

    pre->eh = eh;
    pre->errors = 0;

    return pre;
}

void free_preprocessor(Preprocessor_T* pre)
{
    free_list(pre->functions);
    free_list(pre->typedefs);
    free_list(pre->globals);
    free_list(pre->locals);
    free_list(pre->args);

    free(pre);
}

static ASTTypedef_T* find_typedef(Preprocessor_T* opt, char* tdef)
{
    for(int i = 0; i < opt->typedefs->size; i++)
    {
        ASTTypedef_T* tf = (ASTTypedef_T*) opt->typedefs->items[i];
        if(strcmp(tf->name, tdef) == 0)
            return tf;
    }
    return NULL;
}

static ASTGlobal_T* find_global(Preprocessor_T* opt, char* callee)
{
    for(int i = 0; i < opt->globals->size; i++)
    {
        ASTGlobal_T* gl = (ASTGlobal_T*) opt->globals->items[i];
        if(strcmp(gl->name, callee) == 0)
            return gl;
    }
    return NULL;
}

static ASTLocal_T* find_local(Preprocessor_T* opt, char* callee)
{
    for(int i = 0; i < opt->locals->size; i++)
    {
        ASTLocal_T* loc = (ASTLocal_T*) opt->locals->items[i];
        if(strcmp(loc->name, callee) == 0)
            return loc;
    }
    return NULL;
}

static ASTArgument_T* find_argument(Preprocessor_T* opt, char* callee)
{
    for(int i = 0; i < opt->args->size; i++)
    {
        ASTArgument_T* arg = (ASTArgument_T*) opt->args->items[i];
        if(strcmp(arg->name, callee) == 0)
            return arg;
    }
    return NULL;
}

static ASTFunction_T* find_function(Preprocessor_T* opt, char* callee)
{
    for(int i = 0; i < opt->functions->size; i++)
    {
        ASTFunction_T* fn = (ASTFunction_T*) opt->functions->items[i];
        if(strcmp(fn->name, callee) == 0)
            return fn;
    }
    return NULL;
}

static void register_typedef(Preprocessor_T* opt, ASTTypedef_T* tdef)
{
    ASTTypedef_T* ast = find_typedef(opt, tdef->name);
    if(ast != NULL)
    {
        opt->errors++;

        // TODO: include the file path
        const char* template = "redefinition of type `%s`; first defined:" COLOR_BOLD_WHITE " %ld:%ld" COLOR_RESET;
        char* msg = calloc(strlen(template) + strlen(tdef->name) + 1, sizeof(char));
        sprintf(msg, template, tdef->name, ast->line + 1, ast->pos + 1);
        throw_redef_error(opt->eh, msg, tdef->line, tdef->pos);
        return;
    }        

    list_push(opt->typedefs, tdef);
}

static void register_global(Preprocessor_T* opt, ASTGlobal_T* gl)
{
    ASTGlobal_T* ast = find_global(opt, gl->name);
    if(ast)
    {
        opt->errors++;

        // TODO: include the file path
        const char* template = "redefinition of global variable `%s`; first defined:" COLOR_BOLD_WHITE " %ld:%ld" COLOR_RESET;
        char* msg = calloc(strlen(template) + strlen(gl->name) + 1, sizeof(char));
        sprintf(msg, template, gl->name, ast->line + 1, ast->pos + 1);
        throw_redef_error(opt->eh, msg, gl->line, gl->pos);
        return;
    }

    list_push(opt->globals, gl);
}

static void register_local(Preprocessor_T* opt, ASTLocal_T* loc)
{
    ASTLocal_T* ast = find_local(opt, loc->name);
    if(ast)
    {
        opt->errors++;

        // TODO: include the file path
        const char* template = "redefinition of local variable `%s`; first defined:" COLOR_BOLD_WHITE " %ld:%ld" COLOR_RESET;
        char* msg = calloc(strlen(template) + strlen(loc->name) + 1, sizeof(char));
        sprintf(msg, template, loc->name, ast->line + 1, ast->pos + 1);
        throw_redef_error(opt->eh, msg, loc->line, loc->pos);
        return;
    }

    list_push(opt->locals, loc);
}

static void register_function(Preprocessor_T* opt, ASTFunction_T* fn)
{
    ASTFunction_T* ast = find_function(opt, fn->name);
    if(ast != NULL)
    {
        opt->errors++;

        // TODO: include the file path
        const char* template = "redefinition of function `%s`; first defined:" COLOR_BOLD_WHITE " %ld:%ld" COLOR_RESET;
        char* msg = calloc(strlen(template) + strlen(fn->name) + 1, sizeof(char));
        sprintf(msg, template, fn->name, ast->line + 1, ast->pos + 1);
        throw_redef_error(opt->eh, msg, fn->line, fn->pos);
        return;
    }

    list_push(opt->functions, fn);
}

static void optimize_type(Preprocessor_T* opt, ASTType_T* type)
{
    if(type->type == AST_TYPEDEF)
    {
        if(!find_typedef(opt, type->callee))
        {
            const char* template = "undefined type `%s`";
            char* msg = calloc(strlen(template) + strlen(type->callee) + 1, sizeof(char));
            sprintf(msg, template, type->callee);
            throw_undef_error(opt->eh, msg, type->line, type->pos);
        }
    }

    type->size = type_byte_size_map[type->type];
}

static int type_is_ptr(Preprocessor_T* opt, ASTType_T* type)
{
    int isPtr = type->type == AST_POINTER; //Temporary
    if(type->type == AST_TYPEDEF)
    {
        ASTTypedef_T* tdef = find_typedef(opt, type->callee);
        if(tdef)
        {
            isPtr = tdef->data_type->type == AST_POINTER;
        }
        else
        {
            throw_undef_error(opt->eh, "Undefined type", type->line, type->pos);
        }
    }
    return isPtr;
}

static void optimize_identifier(Preprocessor_T* opt, ASTExpr_T* expr)
{
    ASTIdentifier_T* id = (ASTIdentifier_T*) expr->expr;
    ASTLocal_T* loc = find_local(opt, id->callee);
    if(loc)
    {
        id->isPtr = type_is_ptr(opt, loc->type);
        return; 
    }

    ASTGlobal_T* gl = find_global(opt, id->callee);
    if(gl)
    {
        id->isPtr = type_is_ptr(opt, gl->type);
        return;
    }

    ASTArgument_T* arg = find_argument(opt, id->callee);
    if(arg)
    {
        id->isPtr = type_is_ptr(opt, arg->type);
        return;
    }
}

static void optimize_infix_expr(Preprocessor_T* opt, ASTExpr_T* expr)
{
    ASTInfix_T* ifx = (ASTInfix_T*) expr->expr;

    optimize_expression(opt, ifx->left);
    optimize_expression(opt, ifx->right);   
}

static void optimize_prefix_expr(Preprocessor_T* opt, ASTExpr_T* expr)
{
    ASTPrefix_T* pfx = (ASTPrefix_T*) expr->expr;

    optimize_expression(opt, pfx->right);   
}

static void optimize_postfix_expr(Preprocessor_T* opt, ASTExpr_T* expr)
{
    ASTPostfix_T* pfx = (ASTPostfix_T*) expr->expr;

    optimize_expression(opt, pfx->left);   
}

static void optimize_call_expr(Preprocessor_T* opt, ASTExpr_T* expr)
{
    ASTCall_T* call = (ASTCall_T*) expr->expr;

    for(int i = 0; i < call->args->size; i++)
    {
        optimize_expression(opt, (ASTExpr_T*) call->args->items[i]);
    }
}

static void optimize_index_expr(Preprocessor_T* opt, ASTExpr_T* expr)
{
    ASTIndex_T* idx = (ASTIndex_T*) expr->expr;

    optimize_expression(opt, idx->idx);
    optimize_expression(opt, idx->value); 
}

static void optimize_expression(Preprocessor_T* opt, ASTExpr_T* expr)
{
    switch(expr->type)
    {
        case EXPR_IDENTIFIER:
            optimize_identifier(opt,expr);
            break;
        case EXPR_INFIX:
            optimize_infix_expr(opt, expr);
            break;
        case EXPR_PREFIX:
            optimize_prefix_expr(opt, expr);    
            break;
        case EXPR_POSTFIX:
            optimize_postfix_expr(opt, expr);
            break;
        case EXPR_CALL:
            optimize_call_expr(opt, expr);
            break;
        case EXPR_INDEX:
            optimize_index_expr(opt, expr);
            break;
        
        default:
            break;
    }
}

static void optimize_global(Preprocessor_T* opt, ASTGlobal_T* gl)
{
    optimize_type(opt, gl->type);

    if(gl->value)
        optimize_expression(opt, gl->value);
}

static void optimize_local(Preprocessor_T* opt, ASTLocal_T* loc)
{
    optimize_type(opt, loc->type);

    if(loc->value)
        optimize_expression(opt, loc->value);
}

static void optimize_match(Preprocessor_T* pre, ASTMatch_T* match)
{
    optimize_expression(pre, match->condition);
}

static void optimize_compound(Preprocessor_T* opt, ASTCompound_T* com)
{
    unsigned int local_count = 0;

    for(int i = 0; i < com->stmts->size; i++)
    {
        ASTStmt_T* stmt = (ASTStmt_T*) com->stmts->items[i];
        switch(stmt->type)
        {
        case STMT_LET:
            register_local(opt, (ASTLocal_T*) stmt->stmt);
            local_count++;
            optimize_local(opt, (ASTLocal_T*) stmt->stmt);

            break;
        case STMT_EXPRESSION:
            optimize_expression(opt, ((ASTExprStmt_T*) stmt->stmt)->expr);
            break;
        case STMT_LOOP:
            optimize_expression(opt, ((ASTLoop_T*) stmt->stmt)->condition);
            optimize_compound(opt, ((ASTLoop_T*) stmt->stmt)->body);
            break;
        case STMT_IF:
            optimize_expression(opt, ((ASTIf_T*) stmt->stmt)->condition);
            optimize_compound(opt, ((ASTIf_T*) stmt->stmt)->if_body);
            if(((ASTIf_T*) stmt->stmt)->else_body)
                optimize_compound(opt, ((ASTIf_T*) stmt->stmt)->else_body);
            break;
        case STMT_RETURN:
            optimize_expression(opt, ((ASTReturn_T*) stmt->stmt)->value);
            break;
        case STMT_MATCH:
            optimize_match(opt, (ASTMatch_T*) stmt->stmt);
            break;
        }
    }

    for(; local_count > 0; local_count--)
    {
        opt->locals->size--;
    }
}

static void optimize_function(Preprocessor_T* opt, ASTFunction_T* func)
{
    optimize_type(opt, func->return_type);

    unsigned int argc = 0;
    for(int i = 0; i < func->args->size; i++)
    {
        argc++;
        list_push(opt->args, (ASTArgument_T*) func->args->items[i]);
    }

    optimize_compound(opt, func->body);

    opt->args->size -= argc;
}

static void optimize_ast_file(Preprocessor_T* opt, ASTFile_T* file)
{
    // first, register all contents of the file. This needs to happen first, because the Order of the components does not matter in Spydr
    for(int i = 0; i < file->types->size; i++)
        register_typedef(opt, (ASTTypedef_T*) file->types->items[i]);

    for(int i = 0; i < file->globals->size; i++)
        register_global(opt, (ASTGlobal_T*) file->globals->items[i]);

    for(int i = 0; i < file->functions->size; i++)
        register_function(opt, (ASTFunction_T*) file->functions->items[i]);

    // Optimize everything
    for(int i = 0; i < file->globals->size; i++)
        optimize_global(opt, (ASTGlobal_T*) file->globals->items[i]);

    for(int i = 0; i < file->functions->size; i++)
        optimize_function(opt, (ASTFunction_T*) file->functions->items[i]);
}

void optimize_ast(Preprocessor_T* opt, ASTProgram_T* ast)
{
    for(int i = 0; i < ast->files->size; i++)
    {
        ASTFile_T* fileAST = (ASTFile_T*) ast->files->items[i];

        optimize_ast_file(opt, fileAST);
    }

    if(opt->errors > 0)
    {
        if(opt->errors == 1)
            LOG_ERROR("Encountered 1 error.\n");
        else 
            LOG_ERROR_F("Encountered %d errors.\n", opt->errors);

        free(opt);
        free_ast_program(ast);

        exit(1);
    }
}