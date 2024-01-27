#include "codegen_utils.h"

#include "ast/ast.h"
#include "list.h"
#include "io/log.h"
#include "io/io.h"

#include <libgen.h>
#include <ctype.h>
#include <limits.h>
#include <string.h>
#include <glob.h>
#include <libgen.h>

static void escape_callee(char* dst, ASTIdentifier_T* id)
{
    for(char* c = id->callee; *c; c++)
        if(!isalnum(*c) && *c != '_')
            snprintf(dst + strlen(dst), 4, "$%02x", (int) *c);
        else
            dst[strlen(dst)] = *c;
}

static List_T* get_id_path(ASTIdentifier_T* id) {
    List_T* path = init_list();
    list_push(path, id);

    ASTIdentifier_T* outer = id;
    while(outer->outer)
    {
        outer = outer->outer;
        list_push(path, outer);
    }

    return path;
}

char* gen_identifier(ASTIdentifier_T* id, const char* combiner, const char* prefix)
{
    char* new_c;
    if(id->outer)
    {
        List_T* path = get_id_path(id);

        size_t len = (BUFSIZ) * path->size + 1;
        char callee[len];
        memset(callee, '\0', sizeof callee);

        strcat(callee, prefix);

        for(size_t i = path->size - 1; i > 0; i--)
        {
            escape_callee(callee, path->items[i]);
            strcat(callee, combiner);
        }
        escape_callee(callee, path->items[0]);

        free_list(path);

        new_c = calloc(strlen(callee) + 1, sizeof(char));
        strcpy(new_c, callee);
    }
    else
    {
        new_c = calloc(strlen(id->callee) + strlen(prefix) + 1, sizeof(char));
        strcat(new_c, prefix);
        escape_callee(new_c, id);
    }

    return new_c;
}

bool is_integer(ASTType_T* ty) 
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

bool is_pointer(ASTType_T* ty)
{
    return ty->kind == TY_PTR;
}

static char *find_file(char *pattern) {
    char *path = NULL;
    glob_t buf = {};
    glob(pattern, 0, NULL, &buf);
    if (buf.gl_pathc > 0)
        path = strdup(buf.gl_pathv[buf.gl_pathc - 1]);
    globfree(&buf);
    return path;
}

char* find_libpath(void)
{
    if(file_exists("/usr/lib/x86_64-linux-gnu/crti.o"))
        return "/usr/lib/x86_64-linux-gnu";
    if(file_exists("/usr/lib64/crti.o"))
        return "/usr/lib64";
    
    LOG_ERROR("could not find library path\n");
    exit(1);
}

char* find_gcc_libpath(void)
{
    char* paths[] = {
        "/usr/lib/gcc/x86_64-linux-gnu/*/crtbegin.o",    // default Linux
        "/usr/lib/gcc/x86_64-pc-linux-gnu/*/crtbegin.o", // Gentoo
        "/usr/lib/gcc/x86_64-redhat-linux/*/crtbegin.o"  // Fedora
    };

    for(u64 i = 0; i < sizeof(paths) / sizeof(*paths); i++)
    {
        char* path = find_file(paths[i]);
        if(path)
            return dirname(path);
    }

    LOG_ERROR("gcc library path not found\n");
    exit(1);
}

bool unsigned_type(ASTType_T *ty)
{
    if(!ty)
        return false;
    if(ty->kind == TY_UNDEF)
        return unsigned_type(ty->base);

    return ty->kind == TY_U8 || ty->kind == TY_U16 || ty->kind == TY_U32 || ty->kind == TY_U64;
}

ASTType_T* unpack(ASTType_T* ty)
{
    return ty && ty->kind == TY_UNDEF ? unpack(ty->base) : ty;
}

bool is_variadic(const ASTType_T* ty)
{
    return ty && ty->is_variadic;
}

bool should_emit(Context_T* context, ASTObj_T* obj)
{
    return context->flags.optimize ? obj->referenced : true;
}

bool ptr_type(ASTType_T* ty)
{
    ty = unpack(ty);
    return ty->kind == TY_PTR || ty->kind == TY_VLA;
}

void print_linking_msg(Context_T* context, const char* target, bool is_exec) 
{
    LOG_OK_F(COLOR_BOLD_BLUE "  Linking   " COLOR_RESET " %s " COLOR_BOLD_WHITE "(%s)" COLOR_RESET, basename((char*) target), is_exec ? "executable" : "library");
    if(context->linker_flags->size > 0)
    {
        LOG_OK(COLOR_RESET " (");
        for(size_t i = 0; i < context->linker_flags->size; i++) 
        {
            char* lib = context->linker_flags->items[i];
            if(lib[0] == '-' && lib[1] == 'l') {
                LOG_OK_F(COLOR_RESET "%s%s", (char*) lib + 2, context->linker_flags->size - i <= 1 ? ")" : ", ");
            }
        }
    }
    LOG_OK(COLOR_RESET "\n");
} 
