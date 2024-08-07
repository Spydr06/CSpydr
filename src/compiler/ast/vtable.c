#include "vtable.h"

#include "ast/ast.h"
#include "codegen/codegen_utils.h"
#include "error/error.h"
#include "list.h"
#include "memory/allocator.h"
#include "parser/typechecker.h"

#include <assert.h>
#include <memory.h>

static size_t vtable_id_counter = 0;

ASTVTable_T* ast_vtable_init(Context_T* context, ASTType_T* interface) {
    assert(interface->kind == TY_INTERFACE);

    ASTVTable_T* vtable = allocator_malloc(&context->raw_allocator, sizeof(struct AST_VTABLE_STRUCT));
    vtable->interface = interface;
    vtable->functions = init_list();
    vtable->id = vtable_id_counter++;

    return vtable;
}

ASTImpl_T* ast_impl_init(Context_T* context, ASTType_T* impl_base) {
    assert(impl_base->kind != TY_UNDEF);

    ASTImpl_T* impl = allocator_malloc(&context->raw_allocator, sizeof(struct AST_IMPL_STRUCT));
    impl->base_type = impl_base;
    impl->vtables = init_list();

    return impl;
}

void vtable_register(Context_T* context, ASTProg_T* ast, ASTType_T* interface, ASTType_T* impl_base, ASTObj_T* function) {
    ASTVTable_T* vtable = vtable_get(context, ast, interface, impl_base, true);

    ASTObj_T* existing;
    if((existing = vtable_insert(vtable, function))) {
        char* buf = allocator_malloc(&context->raw_allocator, BUFSIZ * sizeof(char));
        *buf = '\0';

        throw_error(context, ERR_REDEFINITION_UNCR, function->tok, 
            "function `%s` is already implemented for type `%s`.\n"
            "first defined in " COLOR_BOLD_WHITE "%s" COLOR_RESET " at line " COLOR_BOLD_WHITE "%u" COLOR_RESET ".",
            function->id->callee,
            ast_type_to_str(context, buf, impl_base, BUFSIZ - 1),
            EITHER(existing->tok->source->short_path, existing->tok->source->path),
            existing->tok->line + 1
        );
    }
}

ASTVTable_T* vtable_get(Context_T* context, ASTProg_T* ast, ASTType_T* interface, ASTType_T* impl_base, bool create) {
    ASTType_T* impl_base_ext = unpack(impl_base);
    ASTType_T* interface_ext = unpack(interface);

    for(size_t i = 0; i < ast->impls->size; i++) {
        ASTImpl_T* impl = ast->impls->items[i];

        if(!types_equal(context, impl_base_ext, impl->base_type))
            continue;

        for(size_t j = 0; j < impl->vtables->size; j++) {
            ASTVTable_T* vtable = impl->vtables->items[j];

            if(!types_equal(context, vtable->interface, interface_ext))
                continue;

            return vtable;
        }

        if(create) {
            ASTVTable_T* vtable = ast_vtable_init(context, interface_ext);
            list_push(impl->vtables, vtable);
            return vtable;
        }

        return NULL;
    }

    if(create) {
        ASTImpl_T* impl = ast_impl_init(context, impl_base_ext);
        
        ASTVTable_T* vtable = ast_vtable_init(context, interface_ext);
        list_push(impl->vtables, vtable);

        list_push(ast->impls, impl);

        return vtable;
    }

    return NULL;
}

ASTObj_T* vtable_entry(ASTVTable_T* vtable, const char* ident) {
    for(size_t i = 0; i < vtable->functions->size; i++) {
        ASTObj_T* func = vtable->functions->items[i];

        if(strcmp(func->id->callee, ident) == 0)
            return func;
    }

    return NULL;
}

ASTObj_T* vtable_insert(ASTVTable_T* vtable, ASTObj_T* function) {
    assert(function->kind == OBJ_FUNCTION);

    ASTObj_T* existing;
    if((existing = vtable_entry(vtable, function->id->callee)))
        return existing;

    list_push(vtable->functions, function);

    return NULL;
}
