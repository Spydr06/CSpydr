#include "ast.h"
#include "../io/log.h"

#include "types.h"
#include "mem/ast_mem.h"

ASTNode_T* init_ast_node(ASTNodeKind_T kind, Token_T* tok)
{
    ASTNode_T* node = ast_malloc(sizeof(struct AST_NODE_STRUCT));
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

    ast_mem_add_ptr(node->tok);

    return node;
}

void free_ast_node(ASTNode_T* node)
{
}

ASTType_T* init_ast_type(ASTTypeKind_T kind, Token_T* tok)
{
    ASTType_T* type = ast_malloc(sizeof(struct AST_TYPE_STRUCT));
    type->kind = kind;
    type->tok = dupl_token(tok);

    type->size = type_byte_size_map[kind];

    type->base = NULL;
    type->arg_types = NULL;
    type->num_indices = NULL;
    type->members = NULL;

    type->is_primitive = false;
    type->is_constant = false;
    type->size = 0;
    type->is_fn = false;

    ast_mem_add_ptr(type->tok);

    return type;
}

void free_ast_type(ASTType_T* type)
{
}

ASTObj_T* init_ast_obj(ASTObjKind_T kind, Token_T* tok)
{
    ASTObj_T* obj = ast_malloc(sizeof(struct AST_OBJ_STRUCT));
    obj->kind = kind;
    obj->tok = dupl_token(tok);
    obj->is_extern = false;

    obj->data_type = NULL;
    obj->value = NULL;
    obj->return_type = NULL;
    obj->args = NULL;
    obj->body = NULL;

    obj->is_constant = false;

    ast_mem_add_ptr(obj->tok);

    return obj;
}

void free_ast_obj(ASTObj_T* obj)
{
}

ASTProg_T* init_ast_prog(const char* main_file_path, const char* target_binary, List_T* imports)
{
    ASTProg_T* prog = ast_malloc(sizeof(struct AST_PROG_STRUCT));
    prog->main_file_path = main_file_path;
    prog->target_binary = target_binary;

    prog->imports = imports;

    prog->objs = init_list(sizeof(struct AST_OBJ_STRUCT*));
    prog->lambda_literals = init_list(sizeof(struct AST_NODE_STRUCT*));
    prog->tuple_structs = init_list(sizeof(struct AST_TYPE_STRUCT*));

    ast_mem_add_list(prog->objs);
    ast_mem_add_list(prog->lambda_literals);
    ast_mem_add_list(prog->tuple_structs);

    return prog;
}

void free_ast_prog(ASTProg_T* prog)
{
    if(prog == NULL)
        return;

    ast_free();
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