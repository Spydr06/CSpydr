#include "ast.h"
#include "../io/log.h"

#include "types.h"

List_T* freed_ptrs;

#define free_sf(ptr, fn)                                \
{                                                       \
    if(ptr != NULL)                                     \
    {                                                   \
        bool is_freed = false;                          \
        for(size_t i = 0; i < freed_ptrs->size; i++)    \
            if(freed_ptrs->items[i] == ptr)             \
                is_freed = true;                        \
        if(!is_freed)                                   \
        {                                               \
            list_push(freed_ptrs, ptr);                 \
            fn(ptr);                                    \
        }                                               \
    }                                                   \
}

#define free_s(ptr) free_sf(ptr, free)

ASTNode_T* init_ast_node(ASTNodeKind_T kind, Token_T* tok)
{
    ASTNode_T* node = malloc(sizeof(struct AST_NODE_STRUCT));
    node->kind = kind;
    node->tok = dupl_token(tok);
    node->is_default_case = false;
    node->is_constant = false;

    node->int_val = 0;
    node->float_val = 0;
    node->bool_val = false;

    node->is_constant = false;
    node->is_default_case = false;

    node->locals = NULL;
    node->data_type = NULL;
    node->callee = NULL;
    node->str_val = NULL;
    node->left = NULL;
    node->right = NULL;
    node->stmts = NULL;
    node->locals = NULL;
    node->condition = NULL;
    node->if_branch = NULL;
    node->else_branch = NULL;
    node->body = NULL;
    node->return_val = NULL;
    node->cases = NULL;
    node->default_case = NULL;
    node->expr = NULL;
    node->args = NULL;

    return node;
}

void free_ast_node(ASTNode_T* node)
{
    if(!node)
        return;

    free_sf(node->tok, free_token);

    free_s(node->callee);
    free_s(node->str_val);
    free_sf(node->left, free_ast_node);
    free_sf(node->right, free_ast_node);

    free_sf(node->condition, free_ast_node);
    free_sf(node->if_branch, free_ast_node);
    free_sf(node->else_branch, free_ast_node);
    
    if(node->stmts)
    {
        for(size_t i = 0; i < node->stmts->size; i++)
            free_sf(node->stmts->items[i], free_ast_node); 
        free_sf(node->stmts, free_list);
    }
    if(node->locals)
    {
        for(size_t i = 0; i < node->locals->size; i++)
            free_sf(node->locals->items[i], free_ast_obj); 
        free_sf(node->locals, free_list);
    }

    free_sf(node->body, free_ast_node);
    free_sf(node->return_val, free_ast_node);
    free_sf(node->default_case, free_ast_node);
    free_sf(node->expr, free_ast_node);

    if(node->cases)
    {
        for(size_t i = 0; i < node->cases->size; i++)
            free_sf(node->cases->items[i], free_ast_node); 
        free_sf(node->cases, free_list);
    }

    free_s(node);
}

ASTType_T* init_ast_type(ASTTypeKind_T kind, Token_T* tok)
{
    ASTType_T* type = malloc(sizeof(struct AST_TYPE_STRUCT));
    type->kind = kind;
    type->tok = dupl_token(tok);

    type->size = type_byte_size_map[kind];

    type->base = NULL;
    type->callee = NULL;
    type->arg_types = NULL;
    type->num_indices = NULL;
    type->members = NULL;

    type->is_primitive = false;
    type->size = 0;
    type->is_fn = false;

    return type;
}

void free_ast_type(ASTType_T* type)
{
    if(!type || type->is_primitive) // primitives are pre-defined, so they don't need to be freed
        return;

    free_token(type->tok);
    free_sf(type->base, free_ast_type);
    free_s(type->callee);

    if(type->arg_types)
    {
        for(size_t i = 0; i < type->arg_types->size; i++)
            free_sf(type->arg_types->items[i], free_ast_type); 
        free_sf(type->arg_types, free_list);
    }

    free_sf(type->num_indices, free_ast_node);

    if(type->members)
    {
        for(size_t i = 0; i < type->members->size; i++)
            free_sf(type->members->items[i], free_ast_node);
        free_sf(type->members, free_list);
    }

    free_s(type);
}

ASTObj_T* init_ast_obj(ASTObjKind_T kind, Token_T* tok)
{
    ASTObj_T* obj = malloc(sizeof(struct AST_OBJ_STRUCT));
    obj->kind = kind;
    obj->tok = dupl_token(tok);
    obj->is_extern = false;

    obj->callee = NULL;
    obj->data_type = NULL;
    obj->value = NULL;
    obj->return_type = NULL;
    obj->args = NULL;
    obj->body = NULL;

    obj->is_constant = false;

    return obj;
}

void free_ast_obj(ASTObj_T* obj)
{
    if(!obj)
        return;

    free_token(obj->tok);
    
    free_s(obj->callee);
    free_sf(obj->data_type, free_ast_type);
    free_sf(obj->value, free_ast_node);

    free_sf(obj->return_type, free_ast_type);
    if(obj->args)
    {
        for(size_t i = 0; i < obj->args->size; i++)
            free_sf(obj->args->items[i], free_ast_obj);
        free_sf(obj->args, free_list);
    }
    free_sf(obj->body, free_ast_node);

    free_s(obj);
}

ASTProg_T* init_ast_prog(const char* main_file_path, const char* target_binary, List_T* imports)
{
    ASTProg_T* prog = malloc(sizeof(struct AST_PROG_STRUCT));
    prog->main_file_path = main_file_path;
    prog->target_binary = target_binary;

    prog->imports = imports;

    prog->objs = init_list(sizeof(struct AST_OBJ_STRUCT*));
    prog->lambda_literals = init_list(sizeof(struct AST_NODE_STRUCT*));
    prog->tuple_structs = init_list(sizeof(struct AST_TYPE_STRUCT*));

    return prog;
}

void free_ast_prog(ASTProg_T* prog)
{
    if(prog == NULL)
        return;

    freed_ptrs = init_list(sizeof(void*));

    /*for(size_t i = 0; i < prog->objs->size; i++)
        free_ast_obj(prog->objs->items[i]);*/
    free_sf(prog->objs, free_list);

    free_s(prog);

    free_list(freed_ptrs);
}

void merge_ast_progs(ASTProg_T* dest, ASTProg_T* src)
{
    for(size_t i = 0; i < src->objs->size; i++)
        list_push(dest->objs, src->objs->items[i]);

    free_list(src->objs);
    free(src);
}

const char* obj_kind_to_str(ASTObjKind_T kind)
{
    switch(kind)
    {
        case OBJ_FUNCTION:
            return "function";
        case OBJ_TYPEDEF:
            return "typedef";
        case OBJ_LOCAL: 
            return "local variable";
        case OBJ_GLOBAL:
            return "global variable";
        case OBJ_FN_ARG:
            return "argument";
    }
    return "NULL";
}