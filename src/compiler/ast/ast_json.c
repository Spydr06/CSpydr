#include "ast_json.h"
#include "ast.h"
#include "../io/io.h"

#include <string.h>
#include <json-c/json.h>
#include <json-c/json_types.h>

typedef json_object* (*IndexFn)(void*);

json_object* gen_ast_prog(ASTProg_T* ast);
json_object* gen_ast_type(ASTType_T* type);
json_object* gen_ast_obj(ASTObj_T* obj);

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

    json_object_object_add(obj, "value", gen_str(tok->value));
    json_object_object_add(obj, "line", gen_i64(tok->line));
    json_object_object_add(obj, "pos", gen_i64(tok->line));
    json_object_object_add(obj, "type", gen_i64(tok->line));
    json_object_object_add(obj, "file_no", gen_i32(tok->source ? tok->source->file_no : -1));

    return obj;
}

json_object* gen_srcfile(SrcFile_T* file)
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

    json_object_object_add(obj, "kind", gen_i32(id->kind));
    json_object_object_add(obj, "tok", gen_tok(id->tok));
    json_object_object_add(obj, "callee", gen_str(id->callee));
    json_object_object_add(obj, "outer", gen_ast_identifier(id->outer));
    json_object_object_add(obj, "global_scope", gen_bool(id->global_scope));

    return obj;
}

json_object* gen_ast_node(ASTNode_T* node)
{
    if(!node)
        return json_object_new_null();
    json_object* obj = json_object_new_object();

    json_object_object_add(obj, "kind", gen_i32(node->kind));
    json_object_object_add(obj, "tok", gen_tok(node->tok));
    json_object_object_add(obj, "data_type", gen_ast_type(node->data_type));
    json_object_object_add(obj, "id", gen_ast_identifier(node->id));
    // todo: unions
    json_object_object_add(obj, "left", gen_ast_node(node->left));
    json_object_object_add(obj, "right", gen_ast_node(node->right));
    json_object_object_add(obj, "is_ptr", gen_bool(node->is_ptr));
    json_object_object_add(obj, "stmts", gen_list(node->stmts, (IndexFn) gen_ast_node));
    json_object_object_add(obj, "locals", gen_list(node->locals, (IndexFn) gen_ast_obj));
    json_object_object_add(obj, "condition", gen_ast_node(node->condition));
    json_object_object_add(obj, "if_branch", gen_ast_node(node->if_branch));
    json_object_object_add(obj, "else_branch", gen_ast_node(node->else_branch));
    json_object_object_add(obj, "body", gen_ast_node(node->body));
    json_object_object_add(obj, "init_stmt", gen_ast_node(node->init_stmt));
    // todo: unions
    json_object_object_add(obj, "cases", gen_list(node->cases, (IndexFn) gen_ast_node));
    json_object_object_add(obj, "default_case", gen_ast_node(node->default_case));
    // todo: unions
    json_object_object_add(obj, "is_constant", gen_bool(node->is_constant));
    json_object_object_add(obj, "expr", gen_ast_node(node->expr));
    json_object_object_add(obj, "the_type", gen_ast_type(node->the_type));
    json_object_object_add(obj, "args", gen_list(node->args, (IndexFn) gen_ast_node));

    return obj;
}

json_object* gen_ast_type(ASTType_T* type)
{
    if(!type)
        return json_object_new_null();
    json_object* obj = json_object_new_object();

    json_object_object_add(obj, "kind", gen_i32(type->kind));
    json_object_object_add(obj, "tok", gen_tok(type->tok));
    json_object_object_add(obj, "base", gen_ast_type(type->base));
    json_object_object_add(obj, "size", gen_i32(type->size));
    json_object_object_add(obj, "id", gen_ast_identifier(type->id));
    json_object_object_add(obj, "is_primitive", gen_bool(type->is_primitive));
    json_object_object_add(obj, "is_constant", gen_bool(type->is_constant));
    json_object_object_add(obj, "is_complex", gen_bool(type->is_complex));
    json_object_object_add(obj, "is_atomic", gen_bool(type->is_atomic));
    json_object_object_add(obj, "is_fn", gen_bool(type->is_fn));
    json_object_object_add(obj, "is_union", gen_bool(type->is_union));
    json_object_object_add(obj, "is_volatile", gen_bool(type->is_volatile));
    json_object_object_add(obj, "arg_types", gen_list(type->arg_types, (IndexFn) gen_ast_type));
    json_object_object_add(obj, "num_indices", gen_ast_node(type->num_indices));
    json_object_object_add(obj, "members", gen_list(type->members, (IndexFn) gen_ast_node));

    return obj;
}

json_object* gen_ast_obj(ASTObj_T* obj)
{
    if(!obj)
        return json_object_new_null();
    json_object* jobj = json_object_new_object();

    json_object_object_add(jobj, "kind", gen_i32(obj->kind));
    json_object_object_add(jobj, "tok", gen_tok(obj->tok));
    json_object_object_add(jobj, "id", gen_ast_identifier(obj->id));
    json_object_object_add(jobj, "offset", gen_i32(obj->offset));
    json_object_object_add(jobj, "stack_size", gen_i32(obj->stack_size));
    json_object_object_add(jobj, "align", gen_i32(obj->align));
    json_object_object_add(jobj, "is_constant", gen_bool(obj->is_constant));
    json_object_object_add(jobj, "is_extern", gen_bool(obj->is_extern));
    json_object_object_add(jobj, "referenced", gen_bool(obj->referenced));
    json_object_object_add(jobj, "data_type", gen_ast_type(obj->data_type));
    json_object_object_add(jobj, "value", gen_ast_node(obj->value));
    json_object_object_add(jobj, "return_type", gen_ast_type(obj->return_type));
    json_object_object_add(jobj, "args", gen_list(obj->args, (IndexFn) gen_ast_obj));
    json_object_object_add(jobj, "body", gen_ast_node(obj->body));
    json_object_object_add(jobj, "alloca_size", gen_ast_obj(obj->alloca_size));
    json_object_object_add(jobj, "alloca_bottom", gen_ast_obj(obj->alloca_bottom));
    json_object_object_add(jobj, "objs", gen_list(obj->objs, (IndexFn) gen_ast_obj));

    return jobj;
}

json_object* gen_ast_prog(ASTProg_T* ast)
{   
    json_object* obj = json_object_new_object();
    json_object_object_add(obj, "main_file_path", gen_str(ast->main_file_path));
    json_object_object_add(obj, "target_binary", gen_str(ast->target_binary));
    
    json_object_object_add(obj, "imports", gen_list(ast->imports, (IndexFn) gen_srcfile));
    json_object_object_add(obj, "objs", gen_list(ast->objs, (IndexFn) gen_ast_obj));
    json_object_object_add(obj, "lambda_literals", gen_list(ast->lambda_literals, (IndexFn) gen_ast_obj));
    json_object_object_add(obj, "tuple_structs", gen_list(ast->tuple_structs, (IndexFn) gen_ast_obj));

    return obj;
}



void ast_from_json(ASTProg_T* ast, const char* file)
{

}