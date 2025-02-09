#include "context.h"
#include "config.h"
#include "hashmap.h"
#include "list.h"
#include "memory/allocator.h"

#include <string.h>

Flags_T default_flags(void)
{
    Flags_T flags = (Flags_T){.flags = 0};
    flags.optimize = true;
    flags.embed_debug_info = true;
    flags.do_linking = true;
    flags.do_assembling = true;
    flags.require_entrypoint = true;
    return flags;
}

void init_context(Context_T* context)
{
    memset(context, 0, sizeof(Context_T));
    
    init_allocator(&context->raw_allocator, free);
    init_allocator(&context->list_allocator, (void (*)(void*)) free_list);
    init_allocator(&context->hashmap_allocator, (void (*)(void*)) hashmap_free);
    context->flags = default_flags();

    context->max_macro_call_depth = __CSP_DEFAULT_MAX_MACRO_CALL_DEPTH;
    
    context->compiler_flags = init_list();
    link_mode_init_default(&context->link_mode);

    context->included_libs = hashmap_init();
    context->paths.std_path = DEFAULT_STD_PATH;

    context->cc = DEFAULT_CC;
    context->as = DEFAULT_ASSEMBLER;
    context->ld = DEFAULT_LINKER;
}

void free_context(Context_T *context)
{
    hashmap_free(context->included_libs);
    free_allocator(&context->raw_allocator);
    free_allocator(&context->list_allocator);
    free_allocator(&context->raw_allocator);
    link_mode_free(&context->link_mode);
}

void context_free_allocators(Context_T* context)
{
    free_allocator(&context->raw_allocator);
    free_allocator(&context->list_allocator);
    free_allocator(&context->raw_allocator);
}

void link_mode_init_default(LinkMode_T* link_mode)
{
    memset(link_mode, 0, sizeof(struct CSPYDR_LINKMODE_STRUCT));
    link_mode->mode = LINK_DYNAMIC;
    link_mode->ldynamic.dynamic_linker = CSPYDR_DEFAULT_DYNAMIC_LINKER_PATH;
    link_mode->libs = init_list();
    link_mode->extra = init_list();
}

void link_mode_free(LinkMode_T* link_mode)
{
    free_list(link_mode->libs);
    free_list(link_mode->extra);
}

