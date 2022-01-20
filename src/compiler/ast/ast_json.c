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

    if(strlen(tok->value)) json_object_object_add(obj, "value", gen_str(tok->value));
    if(tok->line) json_object_object_add(obj, "line", gen_i64(tok->line));
    if(tok->pos) json_object_object_add(obj, "pos", gen_i64(tok->pos));
    json_object_object_add(obj, "type", gen_i64(tok->type));
    if(tok->source) json_object_object_add(obj, "file_no", gen_i32(tok->source->file_no));

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
    if(id->tok) json_object_object_add(obj, "tok", gen_tok(id->tok));
    if(strlen(id->callee)) json_object_object_add(obj, "callee", gen_str(id->callee));
    if(id->outer) json_object_object_add(obj, "outer", gen_ast_identifier(id->outer));
    if(id->global_scope) json_object_object_add(obj, "global_scope", gen_bool(id->global_scope));

    return obj;
}

json_object* gen_ast_node(ASTNode_T* node)
{
    if(!node)
        return json_object_new_null();
    json_object* obj = json_object_new_object();

    json_object_object_add(obj, "kind", gen_i32(node->kind));
    if(node->tok) json_object_object_add(obj, "tok", gen_tok(node->tok));
    if(node->data_type) json_object_object_add(obj, "data_type", gen_ast_type(node->data_type));
    if(node->id) json_object_object_add(obj, "id", gen_ast_identifier(node->id));
    // todo: unions
    if(node->left) json_object_object_add(obj, "left", gen_ast_node(node->left));
    if(node->right) json_object_object_add(obj, "right", gen_ast_node(node->right));
    if(node->stmts) json_object_object_add(obj, "stmts", gen_list(node->stmts, (IndexFn) gen_ast_node));
    if(node->locals) json_object_object_add(obj, "locals", gen_list(node->locals, (IndexFn) gen_ast_obj));
    if(node->condition) json_object_object_add(obj, "condition", gen_ast_node(node->condition));
    if(node->if_branch) json_object_object_add(obj, "if_branch", gen_ast_node(node->if_branch));
    if(node->else_branch) json_object_object_add(obj, "else_branch", gen_ast_node(node->else_branch));
    if(node->body) json_object_object_add(obj, "body", gen_ast_node(node->body));
    if(node->init_stmt) json_object_object_add(obj, "init_stmt", gen_ast_node(node->init_stmt));
    // todo: unions
    if(node->cases) json_object_object_add(obj, "cases", gen_list(node->cases, (IndexFn) gen_ast_node));
    if(node->default_case) json_object_object_add(obj, "default_case", gen_ast_node(node->default_case));
    // todo: unions
    if(node->is_constant) json_object_object_add(obj, "is_constant", gen_bool(node->is_constant));
    if(node->expr) json_object_object_add(obj, "expr", gen_ast_node(node->expr));
    if(node->the_type) json_object_object_add(obj, "the_type", gen_ast_type(node->the_type));
    if(node->args) json_object_object_add(obj, "args", gen_list(node->args, (IndexFn) gen_ast_node));

    return obj;
}

json_object* gen_ast_type(ASTType_T* type)
{
    if(!type)
        return json_object_new_null();
    json_object* obj = json_object_new_object();

    json_object_object_add(obj, "kind", gen_i32(type->kind));
    if(type->tok) json_object_object_add(obj, "tok", gen_tok(type->tok));
    if(type->base) json_object_object_add(obj, "base", gen_ast_type(type->base));
    if(type->size) json_object_object_add(obj, "size", gen_i32(type->size));
    if(type->align) json_object_object_add(obj, "align", gen_i32(type->align));
    if(type->id) json_object_object_add(obj, "id", gen_ast_identifier(type->id));
    if(type->is_primitive) json_object_object_add(obj, "is_primitive", gen_bool(type->is_primitive));
    if(type->is_constant) json_object_object_add(obj, "is_constant", gen_bool(type->is_constant));
    if(type->is_complex) json_object_object_add(obj, "is_complex", gen_bool(type->is_complex));
    if(type->is_atomic) json_object_object_add(obj, "is_atomic", gen_bool(type->is_atomic));
    if(type->is_fn) json_object_object_add(obj, "is_fn", gen_bool(type->is_fn));
    if(type->is_union) json_object_object_add(obj, "is_union", gen_bool(type->is_union));
    if(type->is_volatile) json_object_object_add(obj, "is_volatile", gen_bool(type->is_volatile));
    if(type->arg_types) json_object_object_add(obj, "arg_types", gen_list(type->arg_types, (IndexFn) gen_ast_type));
    if(type->num_indices) json_object_object_add(obj, "num_indices", gen_ast_node(type->num_indices));
    if(type->members) json_object_object_add(obj, "members", gen_list(type->members, (IndexFn) gen_ast_node));

    return obj;
}

json_object* gen_ast_obj(ASTObj_T* obj)
{
    if(!obj)
        return json_object_new_null();
    json_object* jobj = json_object_new_object();

    json_object_object_add(jobj, "kind", gen_i32(obj->kind));
    if(obj->tok) json_object_object_add(jobj, "tok", gen_tok(obj->tok));
    if(obj->id) json_object_object_add(jobj, "id", gen_ast_identifier(obj->id));
    if(obj->offset) json_object_object_add(jobj, "offset", gen_i32(obj->offset));
    if(obj->stack_size) json_object_object_add(jobj, "stack_size", gen_i32(obj->stack_size));
    if(obj->is_constant) json_object_object_add(jobj, "is_constant", gen_bool(obj->is_constant));
    if(obj->is_extern) json_object_object_add(jobj, "is_extern", gen_bool(obj->is_extern));
    if(obj->referenced) json_object_object_add(jobj, "referenced", gen_bool(obj->referenced));
    if(obj->data_type) json_object_object_add(jobj, "data_type", gen_ast_type(obj->data_type));
    if(obj->value) json_object_object_add(jobj, "value", gen_ast_node(obj->value));
    if(obj->return_type) json_object_object_add(jobj, "return_type", gen_ast_type(obj->return_type));
    if(obj->args) json_object_object_add(jobj, "args", gen_list(obj->args, (IndexFn) gen_ast_obj));
    if(obj->body) json_object_object_add(jobj, "body", gen_ast_node(obj->body));
    if(obj->alloca_size) json_object_object_add(jobj, "alloca_size", gen_ast_obj(obj->alloca_size));
    if(obj->alloca_bottom) json_object_object_add(jobj, "alloca_bottom", gen_ast_obj(obj->alloca_bottom));
    if(obj->objs) json_object_object_add(jobj, "objs", gen_list(obj->objs, (IndexFn) gen_ast_obj));

    return jobj;
}

json_object* gen_ast_prog(ASTProg_T* ast)
{   
    json_object* obj = json_object_new_object();
    json_object_object_add(obj, "main_file_path", gen_str(ast->main_file_path));
    json_object_object_add(obj, "target_binary", gen_str(ast->target_binary));
    
    json_object_object_add(obj, "imports", gen_list(ast->imports, (IndexFn) gen_srcfile));
    json_object_object_add(obj, "objs", gen_list(ast->objs, (IndexFn) gen_ast_obj));
    json_object_object_add(obj, "tuple_structs", gen_list(ast->tuple_structs, (IndexFn) gen_ast_obj));

    return obj;
}



void ast_from_json(ASTProg_T* ast, const char* file)
{

}