#include "xml.h"
#include "../../../lib/LittleXML/lxml.h"
#include "../io/io.h"
#include "../io/log.h"
#include "../platform/platform_bindings.h"

const char* xml_header = "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n";

void parse_type(ASTType_T* type, XMLNode* parent, char* tag) {
    XMLNode* node = XMLNode_new(parent);
    node->tag = strdup(tag);

    XMLAttribute* is_prim = malloc(sizeof(XMLAttribute));
    is_prim->key = strdup("is-primitive");
    is_prim->value = strdup(type->is_primitive ? "true" : "false");
    XMLAttributeList_add(&node->attributes, is_prim);

    switch(type->kind) {
        case TY_UNDEF:
            node->inner_text = type->callee;
            break;
        case TY_PTR: {
            XMLNode* ptr = XMLNode_new(node);
            ptr->tag = strdup("pointer");
            parse_type(type->base, ptr, "base");
            } break;
        case TY_ARR: {
            XMLNode* arr = XMLNode_new(node);
            arr->tag = strdup("array");
            parse_type(type->base, arr, "base");
        } break;
        case TY_ENUM: {
            XMLNode* _enum = XMLNode_new(node);
            _enum->tag = strdup("enum");
            for(size_t i = 0; i < type->members->size; i++) {
                XMLNode* member = XMLNode_new(_enum);
                member->tag = strdup("member");

                XMLAttribute* callee = malloc(sizeof(XMLAttribute));
                callee->key = strdup("callee");
                callee->value = strdup(((ASTObj_T*) type->members->items[i])->callee);
                XMLAttributeList_add(&node->attributes, callee);
            }        
        } break;
        case TY_STRUCT: {
            XMLNode* _struct = XMLNode_new(node);
            _struct->tag = strdup("struct");
            for(size_t i = 0; i < type->members->size; i++) {
                XMLNode* member = XMLNode_new(_struct);
                member->tag = strdup("member");

                XMLAttribute* callee = malloc(sizeof(XMLAttribute));
                callee->key = strdup("callee");
                callee->value = strdup(((ASTObj_T*) type->members->items[i])->callee);
                XMLAttributeList_add(&node->attributes, callee);

                parse_type(((ASTObj_T*) type->members->items[i])->data_type, member, "data-type");
            }
        } break;
        case TY_TUPLE: {

        } break;
        case TY_I8:
            node->inner_text = "i8";
            break;
        case TY_I16:
            node->inner_text = "i16";
            break;
        case TY_I32:
            node->inner_text = "i32";
            break;
        case TY_I64:
            node->inner_text = "i64";
            break;
        case TY_U8:
            node->inner_text = "u8";
            break;
        case TY_U16:
            node->inner_text = "u16";
            break;
        case TY_U32:
            node->inner_text = "u32";
            break;
        case TY_U64:
            node->inner_text = "u64";
            break;
        case TY_F32:
            node->inner_text = "f32";
            break;
        case TY_F64:
            node->inner_text = "f64";
            break;
        case TY_VOID:
            node->inner_text = "void";
            break;
        case TY_BOOL:
            node->inner_text = "bool";
            break;
        case TY_CHAR:
            node->inner_text = "char";
            break;
        case TY_LAMBDA:
            break;
    }
}

void parse_obj(ASTObj_T* obj, XMLNode* parent) {
    XMLNode* node = XMLNode_new(parent);
    node->tag = strdup(obj->callee);

    XMLAttribute* kind = malloc(sizeof(XMLAttribute));
    kind->key = strdup("kind");
    kind->value = strdup(obj_kind_to_str(obj->kind));
    XMLAttributeList_add(&node->attributes, kind);

    switch(obj->kind) {
        case OBJ_FUNCTION:
            {
                XMLNode* args = XMLNode_new(node);
                args->tag = strdup("arguments");
                for(size_t i = 0; i < obj->args->size; i++)
                    parse_obj(obj->args->items[i], args);

                parse_type(obj->return_type, node, "return-type");
            }
            break;
        case OBJ_GLOBAL: 
            
            break;
        case OBJ_LOCAL:
            
            break;
        case OBJ_TYPEDEF:
            //parse_type(obj->data_type, node, "data-type");
            break;
        case OBJ_FN_ARG:
            
            break;
    }
}

void ast_to_xml(ASTProg_T* ast, const char* path)
{
    write_file(path, (char*) xml_header);

    XMLDocument xml_doc;
    if(!XMLDocument_load(&xml_doc, path)) 
        LOG_ERROR_F("Error reading XML file \"%s\"", path);

    XMLNode* prog = XMLNode_new(xml_doc.root);
    prog->tag = strdup("prog");

    XMLNode* main_file_path = XMLNode_new(prog);
    main_file_path->tag = strdup("main-file-path");
    main_file_path->inner_text = (char*) ast->main_file_path;

    XMLNode* target_bin = XMLNode_new(prog);
    target_bin->tag = strdup("target-binary");
    target_bin->inner_text = (char*) (ast->target_binary ? ast->target_binary : "NULL");

    XMLNode* objs = XMLNode_new(prog);
    objs->tag = strdup("objs");
    for(size_t i = 0; i < ast->objs->size; i++) 
        parse_obj(ast->objs->items[i], objs);

    XMLDocument_write(&xml_doc, path, 4);
    XMLDocument_free(&xml_doc);
}

ASTProg_T* xml_to_ast(const char* path)
{
    return init_ast_prog(path, DEFAULT_OUTPUT_FILE, NULL);
}