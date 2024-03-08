#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#include "ast/ast_iterator.h"
#include "config.h"
#include "context.h"
#include "ast/ast.h"
#include "list.h"
#include "parser/validator.h"

#define GET_VALIDATOR(va) Validator_T* v = va_arg(va, Validator_T*)

static void push_expanded(List_T* dst, ASTType_T* original, ASTType_T* expanded)
{
    list_push(dst, original); // even indices
    list_push(dst, expanded); // uneven indices
}

static ASTType_T* already_expanded(List_T* expanded, ASTType_T* original)
{
    for(size_t i = 0; i < expanded->size; i += 2)
        if(expanded->items[i] == original)
            return expanded->items[i + 1];

    return NULL;
}

static ASTType_T* expand_type_generics_impl(Context_T* context, ASTType_T* type, List_T* generic_objs, List_T* generic_params, List_T* expanded)
{
    switch(type->kind)
    {
        case TY_PTR:
        case TY_ARRAY:
        case TY_VLA:
        case TY_C_ARRAY: {
            ASTType_T* new_base = expand_type_generics_impl(context, type->base, generic_objs, generic_params, expanded);
            if(new_base == type->base)
                return type;

            ASTType_T* copied = malloc(sizeof(ASTType_T));
            CONTEXT_ALLOC_REGISTER(context, (void*) copied);
            memcpy(copied, type, sizeof(ASTType_T));
            
            copied->base = new_base;
            
            return copied;
        }

        case TY_STRUCT: {
            ASTType_T* copied = malloc(sizeof(ASTType_T));
            CONTEXT_ALLOC_REGISTER(context, (void*) copied);
            memcpy(copied, type, sizeof(ASTType_T));

            copied->members = init_list_sized(type->members->size);
            CONTEXT_ALLOC_REGISTER(context, copied->members);

            for(size_t i = 0; i < type->arg_types->size; i++)
            {
                ASTNode_T* member = type->arg_types->items[i];
                ASTNode_T* copied_member = malloc(sizeof(ASTNode_T));
                CONTEXT_ALLOC_REGISTER(context, (void*) copied_member);
                memcpy(copied_member, member, sizeof(ASTNode_T));

                copied_member->data_type = expand_type_generics_impl(context, member->data_type, generic_objs, generic_params, expanded);
                list_push(copied->members, copied_member);
            }

            return copied;
        }
        
        case TY_FN: {
            ASTType_T* copied = malloc(sizeof(ASTType_T));
            CONTEXT_ALLOC_REGISTER(context, (void*) copied);
            memcpy(copied, type, sizeof(ASTType_T));

            copied->base = expand_type_generics_impl(context, type->base, generic_objs, generic_params, expanded);
            copied->arg_types = init_list_sized(type->arg_types->size);
            CONTEXT_ALLOC_REGISTER(context, copied->arg_types);

            for(size_t i = 0; i < type->arg_types->size; i++)
                list_push(copied->arg_types, expand_type_generics_impl(context, copied->arg_types->items[i], generic_objs, generic_params, expanded));
            return copied;
        }

        case TY_UNDEF:
            if(type->referenced_obj && type->referenced_obj->kind == OBJ_TYPEDEF) 
            {
                if(type->generic_params)
                {
                    ASTType_T* cached = already_expanded(expanded, type);
                    if(cached)
                        return cached;

                    List_T* expanded_params = init_list_sized(type->generic_params->size);
                    for(size_t i = 0; i < type->generic_params->size; i++)
                        list_push(expanded_params, expand_type_generics_impl(context, type->generic_params->items[i], generic_objs, generic_params, expanded));
                    
                    ASTType_T* expanded_type = calloc(1, sizeof(ASTType_T));
                    push_expanded(expanded, type, expanded_type);

                    memcpy(expanded_type, expand_type_generics_impl(context, type->base, type->referenced_obj->generics, expanded_params, expanded), sizeof(ASTType_T));

                    free_list(expanded_params);

                    return expanded_type;
                }
                else
                    return type;
            }
            else {
                i64 index = -1;
                for(size_t i = 0; i < generic_objs->size; i++)
                {
                    if(strcmp(((ASTObj_T*)generic_objs->items[i])->id->callee, type->id->callee) == 0)
                    {
                        index = i;
                        break;
                    }
                }

                assert(index != -1);
                return generic_params->items[index];
            }
            break;

        default:
            return type;
    }

}

static ASTType_T* expand_type_generics(Validator_T* v, ASTType_T* type, List_T* generic_objs, List_T* generic_params)
{
    List_T* expanded_types = init_list();
    ASTType_T* expanded = expand_type_generics_impl(v->context, type, generic_objs, generic_params, expanded_types);
    free_list(expanded_types);
    return expanded;
}

static bool is_generic(ASTType_T* type)
{
    switch(type->kind)
    {
        case TY_PTR:
        case TY_ARRAY:
        case TY_VLA:
        case TY_C_ARRAY:
            return is_generic(type->base);

        case TY_FN:
            return is_generic(type->base) || list_any(type->arg_types, (bool (*)(void*)) is_generic);

        case TY_UNDEF:
            return (type->referenced_obj && type->referenced_obj->kind == OBJ_GENERIC) ||
                (type->generic_params && list_any(type->generic_params, (bool (*)(void*)) is_generic));

        case TY_STRUCT:
            for(size_t i = 0; i < type->members->size; i++)
            {
                ASTObj_T* member = type->members->items[i];
                if(is_generic(member->data_type))
                    return true;
            }
            return false;

        default:
            return false;
    }
}

static void expand_generics(ASTType_T* type, va_list args)
{
    GET_VALIDATOR(args);
    if(type->generic_params && !list_any(type->generic_params, (bool (*)(void*))is_generic))
    {
        type->base = expand_type_generics(v, type, type->referenced_obj->generics, type->generic_params);
        free_list(type->generic_params);
        type->generic_params = NULL;
    }
}

void resolve_generics(Validator_T* v)
{
    static const ASTIteratorList_T generic_resolve_iter = {
        .type_fns = {
            [TY_UNDEF] = expand_generics,
        }
    };
    ast_iterate(&generic_resolve_iter, v->ast, v);
}
