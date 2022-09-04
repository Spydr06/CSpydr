#include "codegen_utils.h"

#include "ast/ast.h"
#include "list.h"
#include "mem/mem.h"
#include "io/log.h"
#include "io/io.h"
#include "globals.h"
#include "platform/platform_bindings.h"

#include <libgen.h>
#include <string.h>
#include <glob.h>

static void cat_id(char* callee, ASTIdentifier_T* id)
{
    strcat(callee, id->callee);
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
            cat_id(callee, path->items[i]);
            strcat(callee, combiner);
        }
        cat_id(callee, path->items[0]);

        free_list(path);

        new_c = calloc(strlen(callee) + 1, sizeof(char));
        strcpy(new_c, callee);
    }
    else
    {
        new_c = calloc(strlen(id->callee) + strlen(combiner) + 1, sizeof(char));
        strcat(new_c, combiner);
        strcat(new_c, id->callee);
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

bool is_variadic(ASTType_T* ty)
{
    return ty && ty->is_variadic;
}

bool should_emit(ASTObj_T* obj)
{
    return global.optimize ? obj->referenced : true;
}

bool ptr_type(ASTType_T* ty)
{
    ty = unpack(ty);
    return ty->kind == TY_PTR || ty->kind == TY_VLA;
}

void link_obj(const char* target, char* obj_file, bool silent)
{
    if(!silent)
    {
        LOG_OK_F(COLOR_BOLD_BLUE "  Linking    " COLOR_RESET "%s", target);
        if(global.linker_flags->size > 0)
        {
            LOG_OK(COLOR_RESET " (");
            for(size_t i = 0; i < global.linker_flags->size; i++) 
            {
                char* lib = global.linker_flags->items[i];
                if(lib[0] == '-' && lib[1] == 'l') {
                    LOG_OK_F(COLOR_RESET "%s%s", (char*) lib + 2, global.linker_flags->size - i <= 1 ? ")" : ", ");
                }
            }
        }
        LOG_OK(COLOR_RESET "\n");
    }

    {
        List_T* args = init_list();
        list_push(args, DEFAULT_LINKER);
        list_push(args, "-o");
        list_push(args, (void*) target);
        list_push(args, "-m");
        list_push(args, "elf_x86_64");
        list_push(args, "-L/usr/lib64");
        list_push(args, "-L/lib64");
        list_push(args, "-L/usr/lib");
        list_push(args, "-L/lib");
    
        for(size_t i = 0; i < global.linker_flags->size; i++)
            list_push(args, global.linker_flags->items[i]);
    
        list_push(args, "-dynamic-linker");
        list_push(args, "/lib64/ld-linux-x86-64.so.2");
        list_push(args, obj_file);
        list_push(args, NULL);
    
    
        i32 exit_code = subprocess((char*) args->items[0], (char* const*) args->items, false);
        if(exit_code != 0)
        {
            LOG_ERROR_F("error linking code. (exit code %d)\n", exit_code);
            throw(global.main_error_exception);
        }
    
        free_list(args);
    }
}