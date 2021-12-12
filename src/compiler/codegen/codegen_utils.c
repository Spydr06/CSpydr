#include "codegen_utils.h"

#include "../list.h"
#include "../ast/mem/ast_mem.h"

#include <string.h>

static void cat_id(char* callee, ASTIdentifier_T* id)
{
    strcat(callee, id->callee);
}

static List_T* get_id_path(ASTIdentifier_T* id) {
    List_T* path = init_list(sizeof(struct AST_IDENTIFIER_STRUCT*));
    list_push(path, id);

    ASTIdentifier_T* outer = id;
    while(outer->outer)
    {
        outer = outer->outer;
        list_push(path, outer);
    }

    return path;
}

uint64_t hash_64(const char* key)
{
    uint64_t h = 525201411107845655ull;
    for (;*key;++key) {
        h ^= *key;
        h *= 0x5bd1e9955bd1e995;
        h ^= h >> 47;
    }
    return h;
}


char* gen_identifier(ASTIdentifier_T* id)
{
    if(id->outer == NULL)
        return id->callee;

    static const char* CSP_PREFIX_STR = "__csp_";

    List_T* path = get_id_path(id);

    size_t len = (BUFSIZ) * path->size + 1;
    char callee[len];
    memset(callee, '\0', sizeof callee);
    strcat(callee, CSP_PREFIX_STR);

    for(size_t i = path->size - 1; i > 0; i--)
    {
        cat_id(callee, path->items[i]);
        strcat(callee, "__csp_");
    }
    cat_id(callee, path->items[0]);

    free_list(path);

    char* new_c = calloc(strlen(callee) + 1, sizeof(char));
    sprintf(new_c, "%s", callee);

    if(false)
    {
        if(strlen(new_c) < 20) // bring the string length to the number of digits of UINT64_MAX
            new_c = realloc(new_c, sizeof(char) * 20);
        sprintf(new_c, "%lx", hash_64(new_c));
    }

    ast_mem_add_ptr(new_c);
    return new_c;
}

bool is_integer(ASTType_T *ty) 
{
    ASTTypeKind_T k = ty->kind;
    return k == TY_BOOL || k == TY_CHAR || k == TY_I8 || k == TY_U8 || k == TY_I16 || k == TY_U16 ||
           k == TY_I32 || k == TY_U32  || k == TY_I64 || k == TY_U64 || k == TY_ENUM;
}

bool is_flonum(ASTType_T *ty) 
{
    return ty->kind == TY_F32 || ty->kind == TY_F64 || ty->kind == TY_F80;
}

bool is_numeric(ASTType_T *ty) 
{
    return is_integer(ty) || is_flonum(ty);
}

bool is_unsigned(ASTType_T* ty)
{
    return ty->kind == TY_U8 || ty->kind == TY_U16 || ty->kind == TY_U32 || ty->kind == TY_U64;
}

// Round up `n` to the nearest multiple of `align`.
int align_to(int n, int align) 
{
    return (n + align - 1) / align * align;
}