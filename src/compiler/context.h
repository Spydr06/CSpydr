#ifndef CSPYDR_CONTEXT_H
#define CSPYDR_CONTEXT_H

#include "ast/ast.h"
#include "error/exception.h"
#include "hashmap.h"
#include "list.h"
#include "memory/allocator.h"
#include "util.h"

#define CONTEXT_ALLOC_REGISTER(context, value) do {         \
        allocator_push(&_Generic((value),                   \
                List_T*: (context)->list_allocator,         \
                HashMap_T*: (context)->hashmap_allocator,   \
                void*: (context)->raw_allocator             \
            ),                                              \
            (value)                                         \
        );                                                  \
    } while(0) 

typedef struct BACKEND_STRUCT Backend_T;

typedef enum ARCH_ENUM : u32 {
    ARCH_X86_64  = 0b0001,
    ARCH_AARCH64 = 0b0010,
    ARCH_RISCV64 = 0b0100,
    ARCH_ANY     = UINT32_MAX
} Arch_T;

u32 arch_ptr_size(Arch_T arch);

typedef enum PLATFORM_ENUM : u8 {
    PLATFORM_UNKNOWN      = 0b00000001,
    PLATFORM_FREESTANDING = 0b00000010,
    PLATFORM_LINUX        = 0b00000100,
    PLATFORM_WINDOWS      = 0b00001000,
    PLATFORM_DARWIN       = 0b00010000,
    PLATFORM_BSD          = 0b00100000,
    PLATFORM_ANY          = UINT8_MAX
} Platform_T;

typedef struct TARGET_STRUCT {
    Arch_T arch;
    Platform_T platform;
    const char* libc;
} Target_T;

int parse_target(Target_T* dest, const char* str);
Target_T get_host_target();

Flags_T default_flags(void);

typedef struct CSPYDR_LINKMODE_STRUCT {
    enum {
        LINK_STATIC,
        LINK_DYNAMIC
    } mode;
    union {
        struct {

        } lstatic;
        struct {
            const char* dynamic_linker;
        } ldynamic;
    };

    List_T* libs;
    List_T* extra;
} LinkMode_T;

void link_mode_init_default(LinkMode_T* link_mode);
void link_mode_free(LinkMode_T* link_mode);

typedef struct CSPYDR_CONTEXT_STRUCT {
    i32 fs;

    Target_T target;
    const Backend_T* backend;

    i32 last_exit_code;
    u32 max_macro_call_depth;

    u32 emitted_warnings;
    u32 emitted_errors;

    Flags_T flags;

    struct {
        char* exec_name;
        char* main_src_file;
        char* std_path;
        char* target;
    } paths;

    struct {
        char** argv;
        i32 argc;
    } args;

    char* cc;
    List_T* compiler_flags;

    char* as;

    char* ld;
    LinkMode_T link_mode;
    
    u64 total_source_lines;
    
    ASTObj_T** current_obj;
    Exception_T main_error_exception;

    // list of libraries used in the [link()] directive
    HashMap_T* included_libs;

    Allocator_T raw_allocator;
    Allocator_T list_allocator;
    Allocator_T hashmap_allocator;

    // timesteps recorded by `timer/timer.c`
    List_T* timesteps;
} Context_T;

void init_context(Context_T* context);
void free_context(Context_T* context);

void context_free_allocators(Context_T* context);

#endif
