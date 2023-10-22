#include "context.h"
#include "config.h"
#include "hashmap.h"
#include "list.h"

#include <string.h>

Flags_T default_flags(void)
{
    Flags_T flags = (Flags_T){.flags = 0};
    flags.optimize = true;
    flags.embed_debug_info = true;
    flags.do_linking = true;
    flags.do_assembling = true;
    return flags;
}

void init_context(Context_T* context)
{
    memset(context, 0, sizeof(Context_T));
    
    context->flags = default_flags();

    context->max_macro_call_depth = __CSP_DEFAULT_MAX_MACRO_CALL_DEPTH;
    
    context->compiler_flags = init_list();
    context->linker_flags = init_list();

    context->included_libs = hashmap_init();
    context->paths.std_path = DEFAULT_STD_PATH;

    context->cc = DEFAULT_CC;
    context->cc_flags = DEFAULT_CC_FLAGS;
}

void free_context(Context_T *context)
{
    hashmap_free(context->included_libs);
}