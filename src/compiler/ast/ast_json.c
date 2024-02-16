#include "ast_json.h"
#include "ast.h"
#include "io/io.h"
#include "ast/ast_iterator.h"
#include "list.h"

#include <json-c/json_object.h>
#include <stdarg.h>
#include <string.h>
#include <json-c/json.h>
#include <json-c/json_types.h>

typedef json_object* (*IndexFn)(void*);

json_object* gen_ast_prog(ASTProg_T* ast);
json_object* gen_ast_type(ASTType_T* type);

i32 serializer_pass(Context_T* context, ASTProg_T* ast)
{
    ast_to_json(ast, context->paths.target, context->flags.verbose);
    return 0;
}

void ast_to_json(ASTProg_T* ast, const char* file, bool print_json)
{
    json_object* obj = gen_ast_prog(ast);
    const char* serialized = json_object_to_json_string_ext(obj, JSON_C_TO_STRING_PRETTY);
    if(print_json)
        printf("%s\n", serialized);
    write_file(file, (char*) serialized);
    json_object_put(obj);
}

json_object* gen_str(const char* c)
{
    return c ? json_object_new_string(c) : json_object_new_null();
}

json_object* gen_i32(i32 i)
{
    return json_object_new_int(i);
}

json_object* gen_i64(i64 i)
{
    return json_object_new_int64(i);
}

json_object* gen_bool(bool b)
{
    return json_object_new_boolean(b);
}

json_object* gen_list(List_T* list, IndexFn index_fn)
{
    if(!list)
        return json_object_new_null();
    json_object* arr = json_object_new_array();
    for(size_t i = 0; i < list->size; i++)
        json_object_array_add(arr, index_fn(list->items[i]));
    return arr;
}

json_object* gen_tok(Token_T* tok)
{
    if(!tok)
        return json_object_new_null();
    json_object* obj = json_object_new_object();

    if(strlen(tok->value)) json_object_object_add(obj, "value", gen_str(tok->value));
    if(tok->line) json_object_object_add(obj, "line", gen_i64(tok->line));
    if(tok->pos) json_object_object_add(obj, "pos", gen_i64(tok->pos));
    json_object_object_add(obj, "type", gen_i64(tok->type));
    if(tok->source) json_object_object_add(obj, "file_no", gen_i32(tok->source->file_no));

    return obj;
}

json_object* gen_srcfile(File_T* file)
{
    if(!file)
        return json_object_new_null();
    json_object* obj = json_object_new_object();

    json_object_object_add(obj, "num_lines", gen_i64(file->num_lines));
    json_object_object_add(obj, "file_no", gen_i32(file->file_no));
    //fixme: outputs garbage:
    //json_object_object_add(obj, "path", gen_str(file->path));
    //json_object_object_add(obj, "short_path", gen_str(file->path));

    return obj;
}

json_object* gen_ast_identifier(ASTIdentifier_T* id)
{
    if(!id)
        return json_object_new_null();
    json_object* obj = json_object_new_object();

    if(id->tok) json_object_object_add(obj, "tok", gen_tok(id->tok));
    if(strlen(id->callee)) json_object_object_add(obj, "callee", gen_str(id->callee));
    if(id->outer) json_object_object_add(obj, "outer", gen_ast_identifier(id->outer));
    if(id->global_scope) json_object_object_add(obj, "global_scope", gen_bool(id->global_scope));

    return obj;
}

#define GET_STACK(va) List_T* stack = va_arg(va, List_T*);
#define STACK_TOP ((json_object*) stack->items[stack->size - 1])

void gen_namespace_start(ASTObj_T* namespace, va_list args)
{
    GET_STACK(args);

    list_push(stack, json_object_new_object());
}

void gen_namespace_end(ASTObj_T* namespace, va_list args)
{

}

#undef STACK_TOP
#undef GET_STACK

json_object* gen_ast_prog(ASTProg_T* ast)
{   
    json_object* obj = json_object_new_object();
    json_object_object_add(obj, "main_file_path", gen_str(ast->main_file_path));
    json_object_object_add(obj, "target_binary", gen_str(ast->target_binary));
    
    json_object_object_add(obj, "imports", gen_list(ast->files, (IndexFn) gen_srcfile));
    //json_object_object_add(obj, "objs", gen_list(ast->objs, (IndexFn) gen_ast_obj));
    //json_object_object_add(obj, "tuple_structs", gen_list(ast->tuple_structs, (IndexFn) gen_ast_obj));

    List_T* json_object_stack = init_list();

    ASTIteratorList_T iterator = {
        .obj_start_fns = {
            [OBJ_NAMESPACE] = gen_namespace_start,
        },
        .obj_end_fns = {
            [OBJ_NAMESPACE] = gen_namespace_end,
        }
    };

    ast_iterate(&iterator, ast, json_object_stack);

    return obj;
}



void ast_from_json(ASTProg_T* ast, const char* file)
{

}
