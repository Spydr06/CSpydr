#include "ast.h"
#include "../io/log.h"

#include "types.h"

#define free_sf(ptr, fn) { if(ptr) fn(ptr); ptr = NULL; } // frees a pointer if it's not NULL with a given function

static inline void free_s(void* ptr) free_sf(ptr, free); // frees a pointer if it's not NULL

ASTNode_T* init_ast_node(ASTNodeKind_T kind, Token_T* tok)
{
    ASTNode_T* node = malloc(sizeof(struct AST_NODE_STRUCT));
    node->kind = kind;
    node->tok = dupl_token(tok);
    node->is_default_case = false;
    node->is_constant = false;
    node->locals = NULL;

    return node;
}

void free_ast_node(ASTNode_T* node)
{
    free_sf(node->tok, free_token); // TODO: implement remaining frees

    free_s(node);
}

ASTType_T* init_ast_type(ASTTypeKind_T kind, Token_T* tok)
{
    ASTType_T* type = malloc(sizeof(struct AST_TYPE_STRUCT));
    type->kind = kind;
    type->tok = dupl_token(tok);

    type->size = type_byte_size_map[kind];

    return type;
}

void free_ast_type(ASTType_T* type)
{
    free_sf(type->tok, free_token);

    free_s(type);
}

ASTObj_T* init_ast_obj(ASTObjKind_T kind, Token_T* tok)
{
    ASTObj_T* obj = malloc(sizeof(struct AST_OBJ_STRUCT));
    obj->kind = kind;
    obj->tok = dupl_token(tok);

    return obj;
}

void free_ast_obj(ASTObj_T* obj)
{
    free_sf(obj->tok, free_token);
    
    free_s(obj->callee);
    free_sf(obj->body, free_ast_node)

    free_s(obj);
}

ASTProg_T* init_ast_prog(const char* main_file_path, const char* target_binary)
{
    ASTProg_T* prog = malloc(sizeof(struct AST_PROG_STRUCT));
    prog->main_file_path = main_file_path;
    prog->target_binary = target_binary;

    prog->imports = init_list(sizeof(char*));

    prog->objs = init_list(sizeof(struct AST_OBJ_STRUCT*));

    return prog;
}

void free_ast_prog(ASTProg_T* prog)
{
    for(int i = 0; i < prog->objs->size; i++)
        free_sf(prog->objs->items[i], free_ast_obj);
    free_sf(prog->objs, free_list);

    free_s(prog);
}