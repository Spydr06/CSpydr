#include "ast_iterator.h"
#include "../io/log.h"
#include "ast.h"

#include <stdarg.h>

#define list_fn(fn, ast, ...) if(fn) fn(ast, __VA_ARGS__)

static void ast_obj(ASTIteratorList_T* list, ASTObj_T* obj, va_list custom_args);
static void ast_node(ASTIteratorList_T* list, ASTNode_T* node, va_list custom_args);
static void ast_type(ASTIteratorList_T* list, ASTType_T* type, va_list custom_args);
static void ast_id(ASTIteratorList_T* list, bool is_definition, ASTIdentifier_T* id, va_list custom_args);

void ast_iterate(ASTIteratorList_T* list, ASTProg_T* ast, ...)
{
    va_list custom_args;
    va_start(custom_args, ast);

    // iterate over every object
    for(size_t i = 0; i < ast->objs->size; i++)
    {
        ast_obj(list, ast->objs->items[i], custom_args);
    }

    va_end(custom_args);
}

static void ast_obj(ASTIteratorList_T* list, ASTObj_T* obj, va_list custom_args)
{
    list_fn(list->obj_start_fns[obj->kind], obj, custom_args);
    switch(obj->kind)
    {
        case OBJ_FUNCTION:
            ast_type(list, obj->return_type, custom_args);
            ast_id(list, true, obj->id, custom_args);

            for(size_t i = 0; i < obj->args->size; i++)
                ast_obj(list, obj->args->items[i], custom_args);

            ast_node(list, obj->body, custom_args);     
            break;

        case OBJ_FN_ARG:
            ast_id(list, true, obj->id, custom_args);
            ast_type(list, obj->data_type, custom_args);
            break;

        case OBJ_NAMESPACE:
            ast_id(list, true, obj->id, custom_args);
            
            for(size_t i = 0; i < obj->objs->size; i++)
                ast_obj(list, obj->objs->items[i], custom_args);

            break;

        case OBJ_TYPEDEF:
            ast_id(list, true, obj->id, custom_args);
            ast_type(list, obj->data_type, custom_args);
            break;

        case OBJ_LOCAL:
        case OBJ_GLOBAL:
            ast_id(list, true, obj->id, custom_args);
            ast_type(list, obj->data_type, custom_args);
            ast_node(list, obj->value, custom_args);
            break;

        default:
            // ignore
            break;
    }
    list_fn(list->obj_end_fns[obj->kind], obj, custom_args);
}

static void ast_node(ASTIteratorList_T* list, ASTNode_T* node, va_list custom_args)
{
    switch(node->kind)
    {

    }
}

static void ast_type(ASTIteratorList_T* list, ASTType_T* type, va_list custom_args)
{
    switch(type->kind)
    {
        case TY_I8:
        case TY_I16:
        case TY_I32:
        case TY_I64:
        case TY_U8:
        case TY_U16:
        case TY_U32:
        case TY_U64:
        case TY_F32:
        case TY_F64:
        case TY_F80:
        case TY_BOOL:
        case TY_CHAR:
        case TY_VOID:
        case TY_UNDEF:
        case TY_TEMPLATE: // FIXME: temporary
            list_fn(list->type_fns[type->kind], type, custom_args);
            break;
        
        case TY_PTR:
            ast_type(list, type->base, custom_args);
            list_fn(list->type_fns[TY_PTR], type, custom_args);
            break;

        case TY_ARR:
            ast_type(list, type->base, custom_args);
            if(type->num_indices)
                ast_node(list, type->num_indices, custom_args);
            
            list_fn(list->type_fns[TY_ARR], type, custom_args);
            break;

        case TY_OPAQUE_STRUCT:
            ast_id(list, type->id, false, custom_args);
            list_fn(list->type_fns[TY_OPAQUE_STRUCT], type, custom_args);
            break;

        case TY_ENUM:
            for(size_t i = 0; i < type->members->size; i++)
                ast_node(list, type->members->items[i], custom_args);
            list_fn(list->type_fns[TY_ENUM], type, custom_args);
            break;

        case TY_STRUCT:
            for(size_t i = 0; i < type->members->size; i++)
                ast_node(list, type->members->items[i], custom_args);
            list_fn(list->type_fns[TY_STRUCT], type, custom_args);
            break;
        
        case TY_LAMBDA:
            ast_type(list, type->base, custom_args);
            for(size_t i = 0; i < type->arg_types->size; i++)
                ast_type(list, type->arg_types->items[i], custom_args);
            list_fn(list->type_fns[TY_LAMBDA], type, custom_args);
            break;

        case TY_TUPLE:
            for(size_t i = 0; i < type->arg_types->size; i++)
                ast_type(list, type->arg_types->items[i], custom_args);
            list_fn(list->type_fns[TY_TUPLE], type, custom_args);
            break;
        
        default:
            // ignore
            break;
    }
}

static void ast_id(ASTIteratorList_T* list, bool is_definition, ASTIdentifier_T* id, va_list custom_args)
{
    if(is_definition) 
    {
        list_fn(list->id_def_fn, id, custom_args);
    }
    else
    {
        list_fn(list->id_use_fn, id, custom_args);
    }
}