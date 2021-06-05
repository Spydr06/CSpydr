#include "preprocessor.h"

#include "../io/log.h"
#include "../ast/types.h"

#include <stdio.h>
#include <string.h>

Preprocessor_T* init_preprocessor(void)
{
    Preprocessor_T* pp = malloc(sizeof(struct PREPROCESSOR_STRUCT));
    pp->fns = init_list(sizeof(struct AST_OBJ_STRUCT*));
    pp->vars = init_list(sizeof(struct AST_OBJ_STRUCT*));
    pp->tdefs = init_list(sizeof(struct AST_OBJ_STRUCT*));
    pp->num_errors_found = 0;

    return pp;
}

void free_preprocessor(Preprocessor_T* pp)
{
    free_list(pp->fns);
    free_list(pp->vars);
    free_list(pp->tdefs);

    free(pp);
}

static ASTObj_T* find_fn(Preprocessor_T* pp, char* callee);
static ASTObj_T* find_var(Preprocessor_T* pp, char* callee);
static ASTObj_T* find_tdef(Preprocessor_T* pp, char* callee);

static void register_fn(Preprocessor_T* pp, ASTObj_T* fn);
static void register_var(Preprocessor_T* pp, ASTObj_T* gl);
static void register_tdef(Preprocessor_T* pp, ASTObj_T* ty);

static void check_main_fn(Preprocessor_T* pp);

void preprocess(ASTProg_T* ast)
{
    Preprocessor_T* pp = init_preprocessor();

    for(size_t i = 0; i < ast->objs->size; i++)
    {
        ASTObj_T* obj = ast->objs->items[i];
        switch(obj->kind)
        {
            case OBJ_FUNCTION:
                register_fn(pp, obj);
                break;
            case OBJ_GLOBAL:
                register_var(pp, obj);
                break;
            case OBJ_TYPEDEF:
                register_tdef(pp, obj);
                break;
            default:
                break;
        }
    }
    // TODO: evaluate macros here, when they get added

    check_main_fn(pp);

    if(pp->num_errors_found > 0)
    {
        LOG_ERROR_F("Encountered %d error%s during compilation\n", pp->num_errors_found, pp->num_errors_found == 1 ? "" : "s");
        free_preprocessor(pp);
        exit(1);
    }

    free_preprocessor(pp);
}

static ASTObj_T* find_fn(Preprocessor_T* pp, char* callee)
{
    for(size_t i = 0; i < pp->fns->size; i++)
        if(strcmp(((ASTObj_T*)pp->fns->items[i])->callee, callee) == 0)
            return pp->fns->items[i];
    return NULL;
}

static ASTObj_T* find_var(Preprocessor_T* pp, char* callee)
{
    for(size_t i = 0; i < pp->vars->size; i++)
        if(strcmp(((ASTObj_T*)pp->vars->items[i])->callee, callee) == 0)
            return pp->vars->items[i];
    return NULL;
}

static ASTObj_T* find_tdef(Preprocessor_T* pp, char* callee)
{
    for(size_t i = 0; i < pp->tdefs->size; i++)
        if(strcmp(((ASTObj_T*)pp->tdefs->items[i])->callee, callee) == 0)
            return pp->tdefs->items[i];
    return NULL;
}

static void register_fn(Preprocessor_T* pp, ASTObj_T* fn)
{
    ASTObj_T* found = find_fn(pp, fn->callee);
    if(found) 
    {
        throw_error(ERR_REDEFINITION, fn->tok, "redefinition of function \"%s\", first defined in" COLOR_BOLD_WHITE " %s:[%ld:%ld]" COLOR_RESET, fn->callee, "TODO", found->tok->line, found->tok->pos);
        pp->num_errors_found++;
    }

    list_push(pp->fns, fn);
}

static void register_var(Preprocessor_T* pp, ASTObj_T* var)
{
    ASTObj_T* found = find_var(pp, var->callee);
    if(found)
    {
        throw_error(ERR_REDEFINITION, var->tok, "redefinition of variable \"%s\", first defined in" COLOR_BOLD_WHITE " %s:[%ld:%ld]" COLOR_RESET, var->callee, "TODO", found->tok->line, found->tok->pos);
        pp->num_errors_found++;
    }

    list_push(pp->vars, var);
}

static void register_tdef(Preprocessor_T* pp, ASTObj_T* ty)
{
    if(get_primitive_type(ty->callee))
    {
        throw_error(ERR_REDEFINITION, ty->tok, "redefinition of type \"%s\", cannot overwrite a primitive", ty->callee);
        pp->num_errors_found++;
    }

    ASTObj_T* found = find_tdef(pp, ty->callee);
    if(found)
    {
        throw_error(ERR_REDEFINITION, ty->tok, "redefinition of type \"%s\", first defined in" COLOR_BOLD_WHITE " %s:[%ld:%ld]" COLOR_RESET, ty->callee, "TODO", found->tok->line, found->tok->pos);
        pp->num_errors_found++;
    }
    
    list_push(pp->tdefs, ty);
}

static void check_main_fn(Preprocessor_T* pp)
{
    ASTObj_T* main_fn = find_fn(pp, "main");
    if(!main_fn)
    {
        LOG_ERROR("Missing entry point: could not find function \"main\".\n");
        pp->num_errors_found++;
    }

    if(main_fn->args->size != 0 && main_fn->args->size != 2)
    {
        throw_error(ERR_MISC, main_fn->tok, "arong number of arguments for function \"main\", expect [0, 2]");
        pp->num_errors_found++;
    }

    if(main_fn->return_type->kind != TY_I32)
    {
        throw_error(ERR_MISC, main_fn->return_type->tok, "function \"main\" has to return type `i32`");
        pp->num_errors_found++;
    }

    ASTObj_T* arg1 = (ASTObj_T*)main_fn->args->items[0];
    if(arg1->data_type->kind != TY_I32)
    {
        throw_error(ERR_MISC, arg1->data_type->tok, "argument 1 of function \"main\" has to be of type `i32`");
        pp->num_errors_found++;
    }

    ASTObj_T* arg2 = (ASTObj_T*)main_fn->args->items[1];
    if(arg2->data_type->kind != TY_PTR || arg2->data_type->base->kind != TY_PTR || arg2->data_type->base->base->kind != TY_CHAR)
    {
        throw_error(ERR_MISC, arg2->data_type->tok, "argument 2 of function \"main\" has to be of type `**char`");
        pp->num_errors_found++;
    }
}