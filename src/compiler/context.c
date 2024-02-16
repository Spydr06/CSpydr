#include "context.h"
#include "codegen/backend.h"
#include "config.h"
#include "error/error.h"
#include "hashmap.h"
#include "list.h"
#include "memory/allocator.h"

#include <errno.h>
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

static Arch_T parse_arch(const char** str)
{
    typedef struct { Arch_T a; const char* s; } ArchPair_T;
    static const ArchPair_T PAIRS[] = {
        {ARCH_X86_64, "x86_64"}, {ARCH_AARCH64, "aarch64"}, {ARCH_RISCV64, "riscv64"},
        {0, NULL}
    };

    for(const ArchPair_T* pair = PAIRS; pair->s; pair++)
        if(strncmp(pair->s, *str, strlen(pair->s)) == 0)
        {
            *str += strlen(pair->s);
            return pair->a;
        }
    return ARCH_ANY;
}

static Platform_T parse_platform(const char** str)
{
    typedef struct { Platform_T p; const char* s; } PlatformPair_T;
    static const PlatformPair_T PAIRS[] = {
        {PLATFORM_UNKNOWN, "unknown"}, {PLATFORM_FREESTANDING, "freestanding"}, {PLATFORM_LINUX, "linux"},
        {PLATFORM_WINDOWS, "windows"}, {PLATFORM_DARWIN, "darwin"}, {PLATFORM_BSD, "bsd"},
        {0, NULL}
    };

    for(const PlatformPair_T* pair = PAIRS; pair->s; pair++)
        if(strncmp(pair->s, *str, strlen(pair->s)) == 0)
        {
            *str += strlen(pair->s);
            return pair->p;
        }
    return PLATFORM_ANY;
}

int parse_target(Target_T* dest, const char* str)
{
    dest->arch = parse_arch(&str);
    if(dest->arch == ARCH_ANY)
        return EINVAL;

    if(*str++ != '-')
        return EINVAL;

    dest->platform = parse_platform(&str);
    if(dest->platform == PLATFORM_ANY)
        return EINVAL;

    if(!*str)
    {
        dest->libc = NULL;
        return 0;
    }

    if(*str++ != '-')
        return EINVAL;

    dest->libc = str;
    return 0;
}

Target_T get_host_target()
{
    return (Target_T){
        .arch = ARCH_X86_64, // FIXME: once cspc can run on more arches, figure this out (configure script)!
#ifdef CSPYDR_LINUX
        .platform = PLATFORM_LINUX,
#elif defined(CSPYDR_WINDOWS)
        .platform = PLATFORM_WINDOWS,
#elif defined(CSPYDR_MACOS)
        .platform = PLATFORM_DARWIN,
#elif defined(CSPYDR_BSD)
        .plaform = PLATFORM_BSD,
#else
        .platform = PLATFORM_UNKNOWN,
#endif
        .libc = NULL,
    };
}

u32 arch_ptr_size(Arch_T arch)
{
    switch(arch)
    {
    case ARCH_X86_64:
    case ARCH_AARCH64:
    case ARCH_RISCV64:
        return 8;

    default:
        unreachable();
        return sizeof(void*);
    }
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

    context->backend = NULL;
    context->target = get_host_target();
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

