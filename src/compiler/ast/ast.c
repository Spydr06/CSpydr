#include "ast.h"
#include "error/error.h"

#include "codegen/codegen_utils.h"
#include "config.h"
#include "memory/allocator.h"
#include "optimizer/constexpr.h"
#include "types.h"
#include "context.h"

#include <string.h>
#include <stdio.h>

ASTNode_T* init_ast_node(Allocator_T* alloc, ASTNodeKind_T kind, Token_T* tok)
{
    ASTNode_T* node = allocator_malloc(alloc, sizeof(struct AST_NODE_STRUCT));
    memset(node, 0, sizeof(struct AST_NODE_STRUCT));
    node->kind = kind;
    node->tok = tok;

    return node;
}

ASTIdentifier_T* init_ast_identifier(Allocator_T* alloc, Token_T* tok, char* callee)
{
    ASTIdentifier_T* id = allocator_malloc(alloc, sizeof(struct AST_IDENTIFIER_STRUCT));
    memset(id, 0, sizeof(struct AST_IDENTIFIER_STRUCT));

    id->tok = tok;
    id->global_scope = true;
    id->callee = callee;

    return id;
}

ASTType_T* init_ast_type(Allocator_T* alloc, ASTTypeKind_T kind, Token_T* tok)
{
    ASTType_T* type = allocator_malloc(alloc, sizeof(struct AST_TYPE_STRUCT));
    memset(type, 0, sizeof(struct AST_TYPE_STRUCT));
    
    type->tok = tok;
    type->kind = kind;
    type->size = type_byte_size_map[kind];

    return type;
}

ASTObj_T* init_ast_obj(Allocator_T* alloc, ASTObjKind_T kind, Token_T* tok)
{
    ASTObj_T* obj = allocator_malloc(alloc, sizeof(struct AST_OBJ_STRUCT));
    memset(obj, 0, sizeof(struct AST_OBJ_STRUCT));
    obj->kind = kind;
    obj->tok = tok;
    return obj;
}

void init_ast_prog(Context_T* context, ASTProg_T* prog, const char* main_file_path, const char* target_binary)
{
    prog->main_file_path = main_file_path;
    prog->target_binary = target_binary;

    prog->objs = init_list();
    prog->tuple_structs = init_list();

    CONTEXT_ALLOC_REGISTER(context, prog->objs);
    CONTEXT_ALLOC_REGISTER(context, prog->tuple_structs);
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
        case TY_C_ARRAY:
            return "'c[...]";
        case TY_VLA:
            return "[]";
        case TY_ARRAY:
            return "[...]";
        case TY_STRUCT:
            return "struct";
        case TY_ENUM:
            return "enum";
        case TY_FN:
            return "fn";
        case TY_INTERFACE:
            return "interface";
        case TY_UNDEF:
            return "<undefined>";
        default:
            return "<unknown>";
    }
}

char* ast_type_to_str(Context_T* context, char* dest, const ASTType_T* ty, size_t size)
{
    if(size - strlen(dest) < 32) // if not enough memory is left, return
    {
        strcat(dest, "...");
        return dest;
    }

    if(ty->is_constant)
        strcat(dest, "const ");

    switch(ty->kind)
    {
        case TY_I8...TY_CHAR:
            strcat(dest, type_kind_to_str(ty->kind));
            break;
        case TY_PTR:
            strcat(dest, "&");
            ast_type_to_str(context, dest, ty->base, size);
            break;
        case TY_C_ARRAY:
            ast_type_to_str(context, dest, ty->base, size);
            strcat(dest, " 'c[");
            
            {
                char buf[128] = {'\0'};
                sprintf(buf, "%lu", const_u64(context, ty->num_indices_node));
                strcat(dest, buf);
            }            
            
            strcat(dest, "]");
            break;
        case TY_ARRAY:
            ast_type_to_str(context, dest, ty->base, size);            
            sprintf(dest + strlen(dest), "[%lu]", ty->num_indices);
            break;
        case TY_VLA:
            ast_type_to_str(context, dest, ty->base, size);
            strcat(dest, "[]");
            break;
        case TY_STRUCT:
            strcat(dest, ty->is_union ? "union {" : "struct {");
            for(size_t i = 0; i < ty->members->size; i++)
            {
                ASTNode_T* member = ty->members->items[i];

                switch(member->kind) {
                case ND_STRUCT_MEMBER:
                    if(member->id->callee && strlen(member->id->callee) != 0)
                    {
                        strcat(dest, member->id->callee);
                        strcat(dest, ": ");
                    }

                    ast_type_to_str(context, dest, member->data_type, size);
                    break;

                case ND_EMBED_STRUCT:
                    strcat(dest, "embed ");
                    ast_type_to_str(context, dest, member->data_type, size);
                    break;
                default:
                    unreachable();
                }
                
                if(ty->members->size - i > 1)
                    strcat(dest, ", ");
            }
            strcat(dest, "}");
            break;
        case TY_INTERFACE:
            strcat(dest, "interface {}");
            break;
        case TY_ENUM:
            strcat(dest, "enum");
            break;
        case TY_FN:
            strcat(dest, "fn<");
            ast_type_to_str(context, dest, ty->base, size);
            strcat(dest, ">(");
            for(size_t i = 0; i < ty->arg_types->size; i++)
            {
                ast_type_to_str(context, dest, ty->arg_types->items[i], size);
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

static const char OP_CHARS[ND_KIND_LEN] = {
    [ND_ADD] = '+',
    [ND_SUB] = '-',
    [ND_MUL] = '*',
    [ND_DIV] = '/',
    [ND_MOD] = '%'
};

char operator_char(ASTNode_T* op)
{
    return OP_CHARS[op->kind];
}
