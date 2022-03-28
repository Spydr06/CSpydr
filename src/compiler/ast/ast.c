#include "ast.h"
#include "../io/log.h"

#include "codegen/codegen_utils.h"
#include "config.h"
#include "optimizer/constexpr.h"
#include "types.h"
#include "../mem/mem.h"

#include <string.h>

ASTNode_T* init_ast_node(ASTNodeKind_T kind, Token_T* tok)
{
    ASTNode_T* node = mem_malloc(sizeof(struct AST_NODE_STRUCT));
    memset(node, 0, sizeof(struct AST_NODE_STRUCT));
    node->kind = kind;
    node->tok = tok;

    return node;
}

ASTIdentifier_T* init_ast_identifier(Token_T* tok, char* callee)
{
    ASTIdentifier_T* id = mem_malloc(sizeof(struct AST_IDENTIFIER_STRUCT));
    memset(id, 0, sizeof(struct AST_IDENTIFIER_STRUCT));

    id->tok = tok;
    id->kind = -1;
    id->global_scope = true;
    strcpy(id->callee, callee);

    return id;
}

ASTType_T* init_ast_type(ASTTypeKind_T kind, Token_T* tok)
{
    ASTType_T* type = mem_malloc(sizeof(struct AST_TYPE_STRUCT));
    memset(type, 0, sizeof(struct AST_TYPE_STRUCT));
    
    type->tok = tok;
    type->kind = kind;
    type->size = type_byte_size_map[kind];

    return type;
}

ASTObj_T* init_ast_obj(ASTObjKind_T kind, Token_T* tok)
{
    ASTObj_T* obj = mem_malloc(sizeof(struct AST_OBJ_STRUCT));
    memset(obj, 0, sizeof(struct AST_OBJ_STRUCT));
    obj->kind = kind;
    obj->tok = tok;
    return obj;
}

void init_ast_prog(ASTProg_T* prog, const char* main_file_path, const char* target_binary, List_T* imports)
{
    prog->main_file_path = main_file_path;
    prog->target_binary = target_binary;

    prog->imports = imports;

    prog->objs = init_list(sizeof(struct AST_OBJ_STRUCT*));
    prog->tuple_structs = init_list(sizeof(struct AST_TYPE_STRUCT*));

    mem_add_list(prog->objs);
    mem_add_list(prog->tuple_structs);
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
            return "type";
        case OBJ_LOCAL: 
            return "local variable";
        case OBJ_GLOBAL:
            return "global variable";
        case OBJ_FN_ARG:
            return "function argument";
        case OBJ_NAMESPACE:
            return "namespace";
        case OBJ_ENUM_MEMBER:
            return "enum member";
        case OBJ_LAMBDA:
            return "lambda function";
        default:
            return "<undefined ASTObj_T kind>";
    }
}

char* ast_id_to_str(char* dest, ASTIdentifier_T* id, size_t size)
{
    if(id->outer)
    {
        ast_id_to_str(dest, id->outer, size);
        if(size - strlen(dest) - 1 < 2)
            return dest;
        strcat(dest, "::");
    }
    
    if(size - strlen(dest) - 1 - strlen(id->callee) > 0) // concatenate the callee, if enough memory is left
        strcat(dest, id->callee);
    return dest;
}

const char* type_kind_to_str(ASTTypeKind_T kind)
{
    switch (kind) {
        case TY_I8:
            return "i8";
        case TY_I16:
            return "i16";
        case TY_I32:
            return "i32";
        case TY_I64:
            return "i64";
        case TY_U8:
            return "u8";
        case TY_U16:
            return "u16";
        case TY_U32:
            return "u32";
        case TY_U64:
            return "u64";
        case TY_F32:
            return "f32";
        case TY_F64:
            return "f64";
        case TY_F80:
            return "f80";
        case TY_BOOL:
            return "bool";
        case TY_VOID:
            return "void";
        case TY_CHAR:
            return "char";
        case TY_PTR:
            return "&";
        case TY_ARR:
            return "[]";
        case TY_STRUCT:
            return "struct";
        case TY_ENUM:
            return "enum";
        case TY_FN:
            return "fn";
        case TY_UNDEF:
            return "<undefined>";
        default:
            return "<unknown>";
    }
}

char* ast_type_to_str(char* dest, ASTType_T* ty, size_t size)
{
    if(size - strlen(dest) < 32) // if not enough memory is left, return
        return dest;
    switch(ty->kind)
    {
        case TY_I8...TY_CHAR:
            strcat(dest, type_kind_to_str(ty->kind));
            break;
        case TY_PTR:
            strcat(dest, "&");
            ast_type_to_str(dest, ty->base, size);
            break;
        case TY_ARR:
            ast_type_to_str(dest, ty->base, size);
            strcat(dest, "[");
            if(ty->num_indices)
                sprintf(dest, "%s%ld", dest, const_i64(ty->num_indices));
            strcat(dest, "]");
            break;
        case TY_STRUCT:
            strcat(dest, ty->is_union ? "union {" : "struct {");
            for(size_t i = 0; i < ty->members->size; i++)
            {
                ASTNode_T* member = ty->members->items[i];
                strcat(dest, member->id->callee);
                strcat(dest, ": ");
                ast_type_to_str(dest, member->data_type, size);
                if(ty->members->size - i > 1)
                    strcat(dest, ", ");
            }
            strcat(dest, "}");
            break;
        case TY_ENUM:
            strcat(dest, "enum");
            break;
        case TY_FN:
            strcat(dest, "fn<");
            ast_type_to_str(dest, ty->base, size);
            strcat(dest, ">(");
            for(size_t i = 0; i < ty->arg_types->size; i++)
            {
                ast_type_to_str(dest, ty->arg_types->items[i], size);
                if(ty->arg_types->size - i > 1)
                    strcat(dest, ", ");
            }
            if(is_variadic(ty))
                strcat(dest, ", ...");
            strcat(dest, ")");
            break;
        case TY_UNDEF:
            ast_id_to_str(dest, ty->id, size);
            break;
        case TY_TYPEOF:
            strcat(dest, "<typeof>");
            break;
        default:
            break;
    }

    return dest;
}