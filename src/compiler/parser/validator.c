#include "validator.h"
#include "../ast/ast_iterator.h"
#include "../ast/types.h"
#include "../list.h"
#include "../error/error.h"
#include "../io/log.h"

#include <stdarg.h>
#include <string.h>

#define GET_VALIDATOR(va) Validator_T* v = va_arg(va, Validator_T*)

// validator structs
typedef struct VALIDATOR_SCOPE_STRUCT VScope_T;
struct VALIDATOR_SCOPE_STRUCT
{
    VScope_T* prev;
    List_T* objs;
    ASTIdentifier_T* id;
} __attribute__((packed));

typedef struct VALIDATOR_STRUCT 
{
    VScope_T* current_scope;
    int scope_depth;
    ASTObj_T* current_function;
    bool main_function_found;
} Validator_T;

// validator struct functions
static Validator_T* init_validator(Validator_T* v)
{
    v->scope_depth = 0;
    v->current_scope = NULL;

    return v;
}

static void begin_obj_scope(Validator_T* v, ASTIdentifier_T* id, List_T* objs);
static void scope_add_obj(Validator_T* v, ASTObj_T* obj);
static inline void begin_scope(Validator_T* v, ASTIdentifier_T* id);
static inline void end_scope(Validator_T* v);

// iterator functions

// id
static void id_use(ASTIdentifier_T* id, va_list args);
static void id_def(ASTIdentifier_T* id, va_list args);

// obj
static void fn_start(ASTObj_T* fn, va_list args);
static void fn_end(ASTObj_T* fn, va_list args);
static void namespace_start(ASTObj_T* namespace, va_list args);
static void namespace_end(ASTObj_T* namespace, va_list args);
static void typedef_start(ASTObj_T* tydef, va_list args);
static void typedef_end(ASTObj_T* tydef, va_list args);
static void global_start(ASTObj_T* global, va_list args);
static void global_end(ASTObj_T* global, va_list args);
static void local_start(ASTObj_T* local, va_list args);
static void local_end(ASTObj_T* local, va_list args);
static void fn_arg_start(ASTObj_T* arg, va_list args);
static void fn_arg_end(ASTObj_T* arg, va_list args);

// node
// statements
static void block_start(ASTNode_T* block, va_list args);
static void block_end(ASTNode_T* block, va_list args);
static void return_end(ASTNode_T* ret, va_list args);
static void for_start(ASTNode_T* _for, va_list args);
static void for_end(ASTNode_T* _for, va_list args);
static void case_end(ASTNode_T* _case, va_list args);

// expressions
static void call(ASTNode_T* call, va_list args);
static void identifier(ASTNode_T* id, va_list args);
static void closure(ASTNode_T* closure, va_list args);
static void reference(ASTNode_T* ref, va_list args);
static void dereference(ASTNode_T* deref, va_list args);
static void member(ASTNode_T* member, va_list args);
static void bin_operation(ASTNode_T* op, va_list args);
static void modulo(ASTNode_T* mod, va_list args);
static void negate(ASTNode_T* neg, va_list args);
static void bitwise_negate(ASTNode_T* neg, va_list args);
static void not(ASTNode_T* not, va_list args);
static void equals(ASTNode_T* equals, va_list args);
static void lt_gt(ASTNode_T* lt_gt, va_list args);
static void and_or(ASTNode_T* and_or, va_list args);
static void bitwise_op(ASTNode_T* op, va_list args);
static void inc_dec(ASTNode_T* op, va_list args);
static void index_(ASTNode_T* index, va_list args); // "index" was taken by string.h
static void cast(ASTNode_T* cast, va_list args);

//types
static void struct_type(ASTType_T* s_type, va_list args);
static void enum_type(ASTType_T* e_type, va_list args);

// iterator configuration
static ASTIteratorList_T main_iterator_list = 
{
    .node_start_fns = 
    {
        [ND_BLOCK] = block_start,
        [ND_FOR] = for_start,
    },

    .node_end_fns = 
    {
        // statements
        [ND_BLOCK] = block_end,
        [ND_RETURN] = return_end,
        [ND_FOR] = for_end,
        [ND_CASE] = case_end,

        // expressions
        [ND_ID]      = identifier,
        [ND_CALL]    = call,
        [ND_CLOSURE] = closure,
        [ND_REF]     = reference,
        [ND_DEREF]   = dereference,
        [ND_MEMBER]  = member,
        [ND_ADD]     = bin_operation,
        [ND_SUB]     = bin_operation,
        [ND_MUL]     = bin_operation,
        [ND_DIV]     = bin_operation,
        [ND_MOD]     = modulo,
        [ND_NEG]     = negate,
        [ND_BIT_NEG] = bitwise_negate,
        [ND_NOT]     = not,
        [ND_EQ]      = equals,
        [ND_NE]      = equals,
        [ND_LT]      = lt_gt,
        [ND_LE]      = lt_gt,
        [ND_GT]      = lt_gt,
        [ND_GE]      = lt_gt,
        [ND_AND]     = and_or,
        [ND_OR]      = and_or,
        [ND_XOR]     = and_or,
        [ND_LSHIFT]  = bitwise_op,
        [ND_RSHIFT]  = bitwise_op,
        [ND_BIT_OR]  = bitwise_op,
        [ND_BIT_AND] = bitwise_op,
        [ND_INC]     = inc_dec,
        [ND_DEC]     = inc_dec,
        [ND_INDEX]   = index_,
        [ND_CAST]    = cast,
    },

    .type_fns = 
    {
        [TY_STRUCT] = struct_type,
        [TY_ENUM]   = enum_type,
    },

    .obj_start_fns = 
    {
        [OBJ_FUNCTION]  = fn_start,
        [OBJ_NAMESPACE] = namespace_start,
        [OBJ_TYPEDEF]   = typedef_start,
        [OBJ_GLOBAL]    = global_start,
        [OBJ_LOCAL]     = local_start,
        [OBJ_FN_ARG]    = fn_arg_start,
    },

    .obj_end_fns = 
    {
        [OBJ_FUNCTION]  = fn_end,
        [OBJ_NAMESPACE] = namespace_end,
        [OBJ_TYPEDEF]   = typedef_end,
        [OBJ_GLOBAL]    = global_end,
        [OBJ_LOCAL]     = local_end,
        [OBJ_FN_ARG]    = fn_arg_end,
    },

    .id_def_fn = id_def,
    .id_use_fn = id_use
};

void validate_ast(ASTProg_T* ast)
{
    Validator_T v;
    init_validator(&v);

    begin_obj_scope(&v, NULL, ast->objs);
    ast_iterate(&main_iterator_list, ast, &v);

    end_scope(&v);

    if(!v.main_function_found)
    {
        LOG_ERROR("[ERROR]: mssing entrypoint; no `main` function declared");
        exit(1);
    }
}

static ASTObj_T* search_in_current_scope(VScope_T* scope, char* id)
{
    for(size_t i = 0; i < scope->objs->size; i++)
    {
        ASTObj_T* obj = scope->objs->items[i];
        if(strcmp(obj->id->callee, id) == 0)
            return obj;
    }
    return NULL;
}

static ASTNode_T* search_node_in_current_scope(VScope_T* scope, char* id)
{
    for(size_t i = 0; i < scope->objs->size; i++)
    {
        ASTNode_T* node = scope->objs->items[i];
        if(strcmp(node->id->callee, id) == 0)
            return node;
    }
    return NULL;
}

static ASTObj_T* search_in_scope(VScope_T* scope, char* id)
{
    if(!scope)
        return NULL;

    ASTObj_T* found = search_in_current_scope(scope, id);
    if(found)
        return found;
    return search_in_scope(scope->prev, id);
}

static ASTObj_T* search_id_in_enum(ASTType_T* e_type, ASTIdentifier_T* id)
{
    for(size_t i = 0; i < e_type->members->size; i++)
    {
        ASTObj_T* member = e_type->members->items[i];
        if(strcmp(member->id->callee, id->callee) == 0)
            return member;
    }

    return NULL;
}

static ASTObj_T* search_identifier(VScope_T* scope, ASTIdentifier_T* id)
{
    if(!scope || !id)
        return NULL;

    if(!id->outer)
    {
        ASTObj_T* found = search_in_current_scope(scope, id->callee);
        if(found)
            return found;
        return search_identifier(scope->prev, id);
    }
    else {
        ASTObj_T* outer = search_identifier(scope, id->outer);
        if(!outer)
            return NULL;

        switch(outer->kind)
        {
            case OBJ_NAMESPACE:
                for(size_t i = 0; i < outer->objs->size; i++)
                {
                    ASTObj_T* current = outer->objs->items[i];
                    if(strcmp(current->id->callee, id->callee) == 0)
                        return current;
                }
                break;
            case OBJ_TYPEDEF:
                switch (outer->data_type->kind) {
                    case TY_ENUM:
                    {
                        ASTObj_T* enum_member = search_id_in_enum(outer->data_type, id);
                        if(enum_member)
                            return enum_member;
                    } break; 
                    default:
                        break;
                }
                throw_error(ERR_UNDEFINED, id->outer->tok, "type `%s` has no member called `%s`", outer->id->callee, id->callee);
                break;
            default:
                break;
        }
        return NULL;
    }
}

static void begin_obj_scope(Validator_T* v, ASTIdentifier_T* id, List_T* objs)
{
    begin_scope(v, id);

    for(size_t i = 0; i < objs->size; i++)
        scope_add_obj(v, objs->items[i]);
}

static inline void begin_scope(Validator_T* v, ASTIdentifier_T* id)
{
    VScope_T* scope = malloc(sizeof(VScope_T));
    scope->objs = init_list(sizeof(ASTObj_T*));
    scope->prev = v->current_scope;
    scope->id = id;
    v->current_scope = scope;
}

static inline void end_scope(Validator_T* v)
{
    VScope_T* scope = v->current_scope;
    v->current_scope = scope->prev;
    free_list(scope->objs);
    free(scope);
}

static void scope_add_obj(Validator_T* v, ASTObj_T* obj)
{
    ASTObj_T* found = search_in_current_scope(v->current_scope, obj->id->callee);
    if(found)
    {
        throw_error(ERR_REDEFINITION, obj->id->tok, 
            "redefinition of %s `%s`.\nfirst defined in " COLOR_BOLD_WHITE "%s " COLOR_RESET "at line " COLOR_BOLD_WHITE "%lld" COLOR_RESET " as %s.", 
            obj_kind_to_str(obj->kind), obj->id->callee, 
            found->tok->source->short_path ? found->tok->source->short_path : found->tok->source->path, 
            found->tok->line + 1,
            obj_kind_to_str(found->kind)
        );
        exit(1);
    }
    list_push(v->current_scope->objs, obj);
}

// only used for enum/struct members
static void scope_add_node(Validator_T* v, ASTNode_T* node)
{
    ASTNode_T* found = search_node_in_current_scope(v->current_scope, node->id->callee);
    if(found)
    {
        throw_error(ERR_REDEFINITION, node->id->tok, 
            "redefinition of member `%s`.\nfirst defined in " COLOR_BOLD_WHITE "%s " COLOR_RESET "at line " COLOR_BOLD_WHITE "%lld" COLOR_RESET, 
            node->id->callee, 
            found->tok->source->short_path ? found->tok->source->short_path : found->tok->source->path, 
            found->tok->line + 1
        );
        exit(1);
    }
    list_push(v->current_scope->objs, node); // use the default obj list
}

static ASTType_T* expand_typedef(Validator_T* v, ASTType_T* type)
{
    if(type->kind != TY_UNDEF)
        return type;

    ASTObj_T* ty_def = search_identifier(v->current_scope, type->id);
    if(!ty_def || ty_def->kind != OBJ_TYPEDEF)
    {
        throw_error(ERR_TYPE_ERROR, type->tok, "undefined data type `%s`", type->id->callee);
        return NULL;
    }

    return ty_def->data_type->kind == TY_UNDEF ? expand_typedef(v, ty_def->data_type) : ty_def->data_type;
}

static ASTNode_T* find_member_in_type(Validator_T* v, ASTType_T* type, ASTNode_T* id)
{
    if(id->kind != ND_ID)
        return NULL;

    bool is_ptr = false;

    while(type->kind == TY_PTR || type->kind == TY_UNDEF)
    {
        if(type->kind == TY_PTR)
        {
            if(!is_ptr)
            {
                is_ptr = true;
                type = type->base;
            }
            else
                break;
        }
        else if(type->kind == TY_UNDEF)
            type = expand_typedef(v, type);
    }

    if(type->kind != TY_STRUCT)
    {
        throw_error(ERR_TYPE_ERROR, id->tok, "cannot get member of type `%d`", type->kind);
        return NULL;
    }

    for(size_t i = 0; i < type->members->size; i++)
    {
        ASTNode_T* member = type->members->items[i];

        if(strcmp(member->id->callee, id->id->callee) == 0)
            return member;
    }

    return NULL;
}

static bool is_number(Validator_T* v, ASTType_T* type)
{
    if(!type)
        return false;

    if(type->kind == TY_UNDEF)
        type = expand_typedef(v, type);
    if(!type)
        return false;

    switch(type->kind)
    {
        case TY_I8:
        case TY_I16:
        case TY_I32:
        case TY_I64:
        case TY_U8:
        case TY_U16: 
        case TY_U32:
        case TY_U64:
        case TY_F32:
        case TY_F64:
        case TY_F80:
        case TY_CHAR:
        case TY_ENUM: // enums get at the moment treated as numbers to support operations
            return true;

        default:
            return false;
    }
}

static bool is_integer(Validator_T* v, ASTType_T* type)
{
    if(!type)
        return false;
    
    if(type->kind == TY_UNDEF)
        type = expand_typedef(v, type);
    if(!type)
        return false;

    switch(type->kind) {
        case TY_I8:
        case TY_I16:
        case TY_I32:
        case TY_I64:
        case TY_U8:
        case TY_U16: 
        case TY_U32:
        case TY_U64:
        case TY_CHAR:
        case TY_ENUM:
            return true;

        default: 
            return false;
    }
}

static bool is_ptr(Validator_T* v, ASTType_T* type)
{
    if(!type)
        return false;

    if(type->kind == TY_UNDEF)
        type = expand_typedef(v, type);
    if(!type)
        return false;
    
    return type->kind == TY_PTR;
}

static bool is_bool(Validator_T* v, ASTType_T* type)
{
    if(!type)
        return false;

    if(type->kind == TY_UNDEF)
        type = expand_typedef(v, type);
    if(!type)
        return false;
    
    return type->kind == TY_BOOL;
}

// id

static void id_def(ASTIdentifier_T* id, va_list args)
{
}

static void id_use(ASTIdentifier_T* id, va_list args)
{
    GET_VALIDATOR(args);
    ASTObj_T* found = search_identifier(v->current_scope, id);
    if(!found) {
        throw_error(ERR_UNDEFINED, id->tok, "undefined identifier `%s`.", id->callee);
    }

    id->outer = found->id->outer;
}

static void gen_id_path(VScope_T* v, ASTIdentifier_T* id)
{
    if(!v || !v->id)
        return;

    id->outer = v->id;
    gen_id_path(v->prev, id->outer);
}

// obj

static void check_main_fn(Validator_T* v, ASTObj_T* main_fn)
{
    ASTType_T* return_type = expand_typedef(v, main_fn->return_type);
    if(return_type->kind != TY_I32)
    {
        throw_error(ERR_TYPE_ERROR, main_fn->return_type->tok, "expect type `i32` as return type for function `main`");
        return;
    }
    
    switch(main_fn->args->size)
    {
        case 0:
            return;
        
        case 2:
            // check the types of the two arguments
            {
                ASTType_T* arg0_type = expand_typedef(v, ((ASTObj_T*)main_fn->args->items[0])->data_type);
                if(arg0_type->kind != TY_I32)
                {
                    throw_error(ERR_TYPE_ERROR, ((ASTObj_T*)main_fn->args->items[0])->data_type->tok, "expect first argument of function `main` to be `i32`");
                    return;
                }

                ASTType_T* arg1_type = expand_typedef(v, ((ASTObj_T*)main_fn->args->items[1])->data_type);
                if(arg1_type->kind != TY_PTR || arg1_type->base->kind != TY_PTR || arg1_type->base->base->kind != TY_CHAR)
                {
                    throw_error(ERR_TYPE_ERROR, ((ASTObj_T*)main_fn->args->items[1])->data_type->tok, "expect first argument of function `main` to be `&&char`");
                    return;
                }
            } break;

        default:
            throw_error(ERR_MISC, main_fn->tok, "expect 0 or 2 arguments for function `main`, got %ld", main_fn->args->size);
    }
}

static void fn_start(ASTObj_T* fn, va_list args)
{
    GET_VALIDATOR(args);
    begin_scope(v, fn->id);
    v->current_function = fn;
}

static void fn_end(ASTObj_T* fn, va_list args)
{
    GET_VALIDATOR(args);

    if(strcmp(fn->id->callee, "main") == 0)
    {
        v->main_function_found = true;
        check_main_fn(v, fn);
    }

    end_scope(v);

    gen_id_path(v->current_scope, fn->id);

    v->current_function = NULL;
}

static void namespace_start(ASTObj_T* namespace, va_list args)
{
    GET_VALIDATOR(args);
    begin_obj_scope(v, namespace->id, namespace->objs);
}

static void namespace_end(ASTObj_T* namespace, va_list args)
{
    GET_VALIDATOR(args);
    end_scope(v);
}

static void typedef_start(ASTObj_T* tydef, va_list args)
{
    if(tydef->data_type->kind == TY_ENUM)
        for(size_t i = 0; i < tydef->data_type->members->size; i++)
        {
            ASTObj_T* member = tydef->data_type->members->items[i];
            member->id->outer = tydef->id;
        }
}

static void typedef_end(ASTObj_T* tydef, va_list args)
{
    GET_VALIDATOR(args);

    gen_id_path(v->current_scope, tydef->id);
}

static void global_start(ASTObj_T* global, va_list args)
{

}

static void global_end(ASTObj_T* global, va_list args)
{
    GET_VALIDATOR(args);

    if(!global->data_type)
    {
        if(!global->value->data_type)
        {
            throw_error(ERR_TYPE_ERROR, global->value->tok, "could not resolve datatype for `%s`", global->id->callee);
            return;
        }

        global->data_type = global->value->data_type;
    }

    gen_id_path(v->current_scope, global->id);
}

static void local_start(ASTObj_T* local, va_list args)
{
}

static void local_end(ASTObj_T* local, va_list args)
{
    if(!local->data_type)
    {
        if(!local->value->data_type)
        {
            throw_error(ERR_TYPE_ERROR, local->value->tok, "could not resolve datatype for `%s`", local->id->callee);
            return;
        }

        local->data_type = local->value->data_type;
    }
}

static void fn_arg_start(ASTObj_T* arg, va_list args)
{
    GET_VALIDATOR(args);
    scope_add_obj(v, arg);
}

static void fn_arg_end(ASTObj_T* arg, va_list args)
{

}

// node
// statements

static void block_start(ASTNode_T* block, va_list args)
{
    GET_VALIDATOR(args);
    begin_obj_scope(v, NULL, block->locals);
}

static void block_end(ASTNode_T* block, va_list args)
{
    GET_VALIDATOR(args);
    end_scope(v);
}

static void return_end(ASTNode_T* ret, va_list args)
{
    GET_VALIDATOR(args);
    if(!v->current_function)
    {
        throw_error(ERR_SYNTAX_ERROR, ret->tok, "unexpected return statement outside of function");
        return;
    }

    // type checking already done in the parser
}

static void case_end(ASTNode_T* _case, va_list args)
{
}

static void for_start(ASTNode_T* _for, va_list args)
{
    GET_VALIDATOR(args);
    begin_obj_scope(v, NULL, _for->locals);
}

static void for_end(ASTNode_T* _for, va_list args)
{
    GET_VALIDATOR(args);
    end_scope(v);
}

// expressions

static void call(ASTNode_T* call, va_list args)
{
    GET_VALIDATOR(args);

    ASTObj_T* called_obj = search_identifier(v->current_scope, call->expr->id);
    if(!called_obj)
    {
        throw_error(ERR_UNDEFINED, call->expr->tok, "undefined identifier `%s`");
        return;
    }

    switch(called_obj->kind)
    {
        case OBJ_FUNCTION:
            {
                call->data_type = called_obj->return_type;
                // todo: add argument checking
            } break;
        
        case OBJ_GLOBAL:
        case OBJ_LOCAL:
        case OBJ_FN_ARG:
            {
                if(called_obj->data_type->kind != TY_LAMBDA)
                {
                    throw_error(ERR_TYPE_ERROR, call->expr->tok, "can only call variables of lambda type, got `%d`", called_obj->data_type->kind);
                    return;
                }

                call->data_type = called_obj->data_type->base;
            } break;
        
        default:
            throw_error(ERR_TYPE_ERROR, call->expr->tok, "cannot call %s `%s`", obj_kind_to_str(called_obj->kind), called_obj->id->callee);
            return;
    }
}

static void identifier(ASTNode_T* id, va_list args)
{
    GET_VALIDATOR(args);

    ASTObj_T* refferred_obj = search_identifier(v->current_scope, id->id);
    if(!refferred_obj)
    {
        throw_error(ERR_SYNTAX_WARNING, id->id->tok, "refferring to undefined identifier `%s`", id->id->callee);
        return;
    }

    switch(refferred_obj->kind)
    {
        case OBJ_GLOBAL:
        case OBJ_LOCAL:
        case OBJ_FUNCTION:
        case OBJ_FN_ARG:
        case OBJ_ENUM_MEMBER:
            break;
        
        default:
            throw_error(ERR_TYPE_ERROR, id->id->tok, 
                "identifier `%s` is of kind %s, expect variable or function name", 
                id->id->callee, obj_kind_to_str(refferred_obj->kind)
            );
            return;
    }

    //debug: printf("[%3d: %3d] refferring to %s `%s` with type %d\n", id->tok->line + 1, id->tok->pos + 1, obj_kind_to_str(refferred_obj->kind), refferred_obj->id->callee, refferred_obj->data_type->kind);
    id->data_type = refferred_obj->data_type;
}

static void closure(ASTNode_T* closure, va_list args)
{
    closure->data_type = closure->expr->data_type;
}

static void reference(ASTNode_T* ref, va_list args)
{
    if(!ref->data_type)
    {
        ref->data_type = init_ast_type(TY_PTR, ref->tok);
        ref->data_type->base = ref->right->data_type;
    }
}

static void dereference(ASTNode_T* deref, va_list args)
{
    if(deref->right->data_type->kind != TY_PTR)
    {
        throw_error(ERR_TYPE_ERROR, deref->tok, "can only dereference variables with pointer type");
        return;
    }

    deref->data_type = deref->right->data_type->base;
}

static void member(ASTNode_T* member, va_list args)
{
    if(!member->left->data_type)
    {
        throw_error(ERR_TYPE_ERROR, member->left->tok, "could not resolve data type for `%s`", member->id->callee);
        return;
    }

    GET_VALIDATOR(args);

    ASTNode_T* found = find_member_in_type(v, member->left->data_type, member->right);

    if(!found)
    {
        throw_error(ERR_TYPE_ERROR, member->tok, "type `%d` has no member named `%s`", member->left->data_type->kind, member->right->id->callee);
        return;
    }

    member->data_type = found->data_type;
    member->is_ptr = is_ptr(v, member->left->data_type);
}

static void bin_operation(ASTNode_T* op, va_list args)
{
    GET_VALIDATOR(args);

    if(!is_number(v, op->left->data_type) && !is_ptr(v, op->left->data_type))
    {
        throw_error(ERR_TYPE_ERROR, op->tok, "left: expect integer or pointer type");
        return;
    }

    if(!is_number(v, op->right->data_type) && !is_ptr(v, op->right->data_type))
    {
        throw_error(ERR_TYPE_ERROR, op->tok, "right: expect integer or pointer type");
        return;
    }

    op->data_type = op->right->data_type->size > op->left->data_type->size ? op->right->data_type : op->left->data_type;
}

static void modulo(ASTNode_T* mod, va_list args)
{
    GET_VALIDATOR(args);

    if(!is_integer(v, mod->left->data_type))
    {
        throw_error(ERR_TYPE_ERROR, mod->tok, "left: expect integer type for modulo operation");
        return;
    }

    if(!is_integer(v, mod->right->data_type))
    {
        throw_error(ERR_TYPE_ERROR, mod->tok, "right: expect integer type for modulo operation");
        return;
    }

    mod->data_type = mod->left->data_type;
}

static void negate(ASTNode_T* neg, va_list args)
{
    GET_VALIDATOR(args);

    if(!is_number(v, neg->right->data_type))
    {
        throw_error(ERR_TYPE_ERROR, neg->tok, "can only do bitwise operations on integer types");
        return;
    }

    // todo: change type of unsigned integers
    neg->data_type = neg->right->data_type;
}

static void bitwise_negate(ASTNode_T* neg, va_list args)
{
    GET_VALIDATOR(args);

    if(!is_integer(v, neg->right->data_type))
    {
        throw_error(ERR_TYPE_ERROR, neg->tok, "expect integer type for bitwise negation");
        return;
    }

    neg->data_type = neg->right->data_type;
}

static void not(ASTNode_T* not, va_list args)
{
    not->data_type = (ASTType_T*) primitives[TY_BOOL];
}

static void equals(ASTNode_T* equals, va_list args)
{
    equals->data_type = (ASTType_T*) primitives[TY_BOOL];
}

static void lt_gt(ASTNode_T* op, va_list args)
{
    GET_VALIDATOR(args);

    if(!is_number(v, op->left->data_type) && !is_ptr(v, op->left->data_type))
    {
        throw_error(ERR_TYPE_ERROR, op->tok, "left: expect integer or pointer type");
        return;
    }

    if(!is_number(v, op->right->data_type) && !is_ptr(v, op->right->data_type))
    {
        throw_error(ERR_TYPE_ERROR, op->tok, "right: expect integer or pointer type");
        return;
    }

    op->data_type = (ASTType_T*) primitives[TY_BOOL];
}

static void and_or(ASTNode_T* op, va_list args)
{
    GET_VALIDATOR(args);

    if(!is_bool(v, op->left->data_type))
    {
        throw_error(ERR_TYPE_ERROR, op->tok, "expect boolean type");
        return;
    }

    if(!is_bool(v, op->right->data_type))
    {
        throw_error(ERR_TYPE_ERROR, op->tok, "expect boolean type");
        return;
    }

    op->data_type = op->left->data_type;
}

static void bitwise_op(ASTNode_T* op, va_list args)
{
    GET_VALIDATOR(args);

    if(!is_integer(v, op->left->data_type))
    {
        throw_error(ERR_TYPE_ERROR, op->tok, "left: can only do bitwise operations on integer types");
        return;
    }

    if(!is_integer(v, op->right->data_type))
    {
        throw_error(ERR_TYPE_ERROR, op->tok, "right: can only do bitwise operations with integer types");
        return;
    }

    op->data_type = op->right->data_type->size > op->left->data_type->size ? op->right->data_type : op->left->data_type;
}

static void inc_dec(ASTNode_T* op, va_list args)
{
    GET_VALIDATOR(args);

    if(!is_number(v, op->left->data_type) && !is_ptr(v, op->left->data_type))
    {
        throw_error(ERR_TYPE_ERROR, op->tok, "expect a number type");
        return;
    }

    op->data_type = op->left->data_type;
}

// "index" was taken by string.h
static void index_(ASTNode_T* index, va_list args)
{
    GET_VALIDATOR(args);

    ASTType_T* left_type = expand_typedef(v, index->left->data_type);
    if(!left_type)
        return;

    if(left_type->kind != TY_ARR && left_type->kind != TY_PTR)
    {
        throw_error(ERR_TYPE_ERROR, index->tok, "left: cannot get an index value; wrong type");
        return;
    }

    if(!is_integer(v, index->expr->data_type))
    {
        throw_error(ERR_TYPE_ERROR, index->tok, "index: expect an integer type");
        return;
    }

    index->data_type = index->left->data_type->base;
}   

static void cast(ASTNode_T* cast, va_list args)
{
    //todo: check, if type conversion is valid and safe
}

// types

static void struct_type(ASTType_T* s_type, va_list args)
{
    GET_VALIDATOR(args);
    begin_scope(v, NULL);

    for(size_t i = 0; i < s_type->members->size; i++)
        scope_add_node(v, s_type->members->items[i]);

    end_scope(v);
}

static void enum_type(ASTType_T* e_type, va_list args)
{
    GET_VALIDATOR(args);
    begin_obj_scope(v, NULL, e_type->members);

    end_scope(v);
}