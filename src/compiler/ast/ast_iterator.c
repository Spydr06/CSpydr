#include "ast_iterator.h"
#include "io/log.h"
#include "ast.h"

#include <stdarg.h>
#include <stdio.h>

#define list_fn(fn, ast, custom_args)   \
    do {                                \
        if(fn && ast)                   \
        {                               \
            va_list copy;               \
            va_copy(copy, custom_args); \
            fn(ast, copy);              \
        }                               \
    } while(0)
                          
static void ast_obj(const ASTIteratorList_T* list, ASTObj_T* obj, va_list custom_args);
static void ast_node(const ASTIteratorList_T* list, ASTNode_T* node, va_list custom_args);
static void ast_type(const ASTIteratorList_T* list, ASTType_T* type, va_list custom_args);
static void ast_id(const ASTIteratorList_T* list, bool is_definition, ASTIdentifier_T* id, va_list custom_args);

void ast_iterate(const ASTIteratorList_T* list, ASTProg_T* ast, ...)
{
    va_list custom_args;
    va_start(custom_args, ast);

    // iterate over every object
    for(size_t i = 0; i < ast->objs->size; i++)
        ast_obj(list, ast->objs->items[i], custom_args);

    va_end(custom_args);
}

void ast_iterate_stmt(const ASTIteratorList_T* list, ASTNode_T* stmt, ...)
{
    va_list custom_args;
    va_start(custom_args, stmt);
    ast_node(list, stmt, custom_args);
    va_end(custom_args);
}

static void ast_obj(const ASTIteratorList_T* list, ASTObj_T* obj, va_list custom_args)
{
    if(!obj)
        return;
    
    list_fn(list->obj_start_fns[obj->kind], obj, custom_args);
    switch(obj->kind)
    {
        case OBJ_FUNCTION:
            ast_type(list, obj->return_type, custom_args);
            ast_id(list, true, obj->id, custom_args);

            for(size_t i = 0; i < obj->args->size; i++)
                ast_obj(list, obj->args->items[i], custom_args);

            if(!list->iterate_only_objs)
                ast_node(list, obj->body, custom_args);   
            ast_obj(list, obj->va_area, custom_args);  
            break;

        case OBJ_FN_ARG:
            ast_type(list, obj->data_type, custom_args);
            ast_id(list, true, obj->id, custom_args);
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

        case OBJ_GLOBAL:
            if(!list->iterate_only_objs)
                ast_node(list, obj->value, custom_args);
        case OBJ_LOCAL:
            ast_id(list, true, obj->id, custom_args);
            ast_type(list, obj->data_type, custom_args);
            break;

        default:
            // ignore
            break;
    }
    list_fn(list->obj_end_fns[obj->kind], obj, custom_args);
}

static void ast_node(const ASTIteratorList_T* list, ASTNode_T* node, va_list custom_args)
{
    if(!node)
        return;
    list_fn(list->node_start_fns[node->kind], node, custom_args);

    switch(node->kind)
    {
        case ND_NOOP:
        case ND_BREAK:
        case ND_CONTINUE:
            break;
        
        case ND_USING:
            ast_id(list, false, node->id, custom_args);
            break;
    
        case ND_ID:
            if(node->data_type)
                ast_type(list, node->data_type, custom_args);
            ast_id(list, false, node->id, custom_args);
            break;
    
        case ND_INT:
        case ND_LONG:
        case ND_FLOAT:
        case ND_DOUBLE:
        case ND_BOOL:
        case ND_CHAR:
        case ND_STR:
        case ND_NIL:
            break;
        
        // x op y
        case ND_ADD:
        case ND_SUB:
        case ND_MUL:
        case ND_DIV:
        case ND_MOD:
        case ND_EQ:
        case ND_NE:
        case ND_GT:
        case ND_GE:
        case ND_LT:
        case ND_LE:
        case ND_AND:
        case ND_OR:
        case ND_LSHIFT:
        case ND_RSHIFT:
        case ND_XOR:
        case ND_BIT_OR:
        case ND_BIT_AND:
        case ND_PIPE:
            ast_node(list, node->left, custom_args);
            ast_node(list, node->right, custom_args);
        case ND_HOLE:
            if(node->data_type) 
                ast_type(list, node->data_type, custom_args);
            break;

        case ND_ASSIGN:
            ast_node(list, node->right, custom_args);
            ast_node(list, node->left, custom_args);
            if(node->data_type) 
                ast_type(list, node->data_type, custom_args);
            break;
        
        case ND_CLOSURE:
            ast_node(list, node->expr, custom_args);
            if(node->data_type)
                ast_type(list, node->data_type, custom_args);
            break;

        case ND_ASM:
            for(size_t i = 0; i < node->args->size; i++)
                ast_node(list, node->args->items[i], custom_args);
            break;

        // x.y
        case ND_MEMBER:
            ast_node(list, node->left, custom_args);
            if(node->data_type) 
                ast_type(list, node->data_type, custom_args);
            if(list->iterate_over_right_members)
                ast_node(list, node->right, custom_args);
            break;

        // op x
        case ND_NEG:
        case ND_BIT_NEG:
        case ND_NOT:
        case ND_REF:
        case ND_DEREF:
            ast_node(list, node->right, custom_args);
            if(node->data_type) 
                ast_type(list, node->data_type, custom_args);
            break;
        
        // x op
        case ND_INC:
        case ND_DEC:
            ast_node(list, node->left, custom_args);
            if(node->data_type) 
                ast_type(list, node->data_type, custom_args);
            break;

        case ND_CALL:
            ast_node(list, node->expr, custom_args);
            if(node->data_type) 
                ast_type(list, node->data_type, custom_args);
            for(size_t i = 0; i < node->args->size; i++)
                ast_node(list, node->args->items[i], custom_args);
            break;

        case ND_INDEX:
            ast_node(list, node->left, custom_args);
            ast_node(list, node->expr, custom_args);
            if(node->data_type) 
                ast_type(list, node->data_type, custom_args);
            break;

        case ND_CAST:
            ast_node(list, node->left, custom_args);
            if(node->data_type) 
                ast_type(list, node->data_type, custom_args);
            break;

        case ND_LEN:
            ast_node(list, node->expr, custom_args);
            if(node->data_type) 
                ast_type(list, node->data_type, custom_args);
            break;

        case ND_SIZEOF:
        case ND_ALIGNOF:
            ast_type(list, node->the_type, custom_args);
            break;
        
        case ND_TERNARY:
            ast_node(list, node->condition, custom_args);
            ast_node(list, node->if_branch, custom_args);
            ast_node(list, node->else_branch, custom_args);
            if(node->data_type)
                ast_type(list, node->data_type, custom_args);
            break;
        
        case ND_ELSE_EXPR:
            ast_node(list, node->left, custom_args);
            ast_node(list, node->right, custom_args);
            if(node->data_type)
                ast_type(list, node->data_type, custom_args);
            break;

        case ND_BLOCK:
            for(size_t i = 0; i < node->locals->size; i++)
                ast_obj(list, node->locals->items[i], custom_args);
            for(size_t i = 0; i < node->stmts->size; i++)
                ast_node(list, node->stmts->items[i], custom_args);
            break;

        case ND_IF:
            ast_node(list, node->condition, custom_args);
            ast_node(list, node->if_branch, custom_args);
            ast_node(list, node->else_branch, custom_args);
            break;

        case ND_CASE:
        case ND_WHILE:
            ast_node(list, node->condition, custom_args);
            ast_node(list, node->body, custom_args);
            break;
        
        case ND_WITH:
            ast_obj(list, node->obj, custom_args);
            ast_node(list, node->condition, custom_args);
            ast_node(list, node->if_branch, custom_args);
            ast_node(list, node->else_branch, custom_args);
            break;

        case ND_CASE_TYPE:
            ast_type(list, node->data_type, custom_args);
            ast_node(list, node->body, custom_args);
            break;
        
        case ND_MATCH_TYPE:
            ast_type(list, node->data_type, custom_args);
            ast_node(list, node->body, custom_args);
            for(size_t i = 0; i < node->cases->size; i++)
                ast_node(list, node->cases->items[i], custom_args);
            ast_node(list, node->default_case, custom_args);
            ast_node(list, node->body, custom_args);
            break;

        case ND_LOOP:
            ast_node(list, node->body, custom_args);
            break;

        case ND_FOR:
            for(size_t i = 0; i < node->locals->size; i++)
                ast_obj(list, node->locals->items[i], custom_args);
            if(node->init_stmt)
                ast_node(list, node->init_stmt, custom_args);
            if(node->condition)
                ast_node(list, node->condition, custom_args);
            if(node->expr)
                ast_node(list, node->expr, custom_args);
            ast_node(list, node->body, custom_args);
            break;

        case ND_MATCH:
            ast_node(list, node->condition, custom_args);
            ast_node(list, node->body, custom_args);
            for(size_t i = 0; i < node->cases->size; i++)
                ast_node(list, node->cases->items[i], custom_args);
            ast_node(list, node->body, custom_args);
            ast_node(list, node->default_case, custom_args);
            break;

        case ND_RETURN:
            ast_node(list, node->return_val, custom_args);
            break;

        case ND_EXPR_STMT:
            ast_node(list, node->expr, custom_args);
            break;

        case ND_ARRAY:
        case ND_STRUCT:
            for(size_t i = 0; i < node->args->size; i++)
                ast_node(list, node->args->items[i], custom_args);
            if(node->data_type)
                ast_type(list, node->data_type, custom_args);
            break;

        case ND_STRUCT_MEMBER:
            ast_id(list, true, node->id, custom_args);
            ast_type(list, node->data_type, custom_args);
            break;
        
        case ND_TYPE_EXPR:
            ast_type(list, node->l_type, custom_args);
            ast_type(list, node->r_type, custom_args);
            break;
        
        case ND_LAMBDA:
            for(size_t i = 0; i < node->args->size; i++)
                ast_obj(list, node->args->items[i], custom_args);
            ast_node(list, node->body, custom_args);
            break;
        
        default:
            // ignore
            break;
    }

    list_fn(list->node_end_fns[node->kind], node, custom_args);
}

static void ast_type(const ASTIteratorList_T* list, ASTType_T* type, va_list custom_args)
{
    if(!type)
        return;
    
    list_fn(list->type_begin, type, custom_args);

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
        
        case TY_FN:
            ast_type(list, type->base, custom_args);
            for(size_t i = 0; i < type->arg_types->size; i++)
                ast_type(list, type->arg_types->items[i], custom_args);
            list_fn(list->type_fns[TY_FN], type, custom_args);
            break;
        
        case TY_TYPEOF:
            ast_node(list, type->num_indices, custom_args);
            list_fn(list->type_fns[TY_TYPEOF], type, custom_args);
            break;
        
        default:
            // ignore
            break;
    }

    list_fn(list->type_end, type, custom_args);
}

static void ast_id(const ASTIteratorList_T* list, bool is_definition, ASTIdentifier_T* id, va_list custom_args)
{
    if(is_definition) 
        list_fn(list->id_def_fn, id, custom_args);
    else
        list_fn(list->id_use_fn, id, custom_args);
}