#ifndef CSPYDR_CODEGEN_BACKEND_H
#define CSPYDR_CODEGEN_BACKEND_H

#include "context.h"
#include <stdbool.h>

#define BACKEND_CALLBACK(backend, callback) backend##_##callback
#define BACKEND_CALLBACKS_IMPL(backend)                                      \
    const BackendCallbacks_T backend##_CALLBACKS = ((BackendCallbacks_T) {   \
        .begin_file = backend##_begin_file,                                  \
        .finish_file = backend##_finish_file,                                \
    })

typedef struct TARGET_STRUCT Target_T;
typedef struct CODEGEN_DATA_STRUCT CodegenData_T;

typedef struct BACKEND_CALLBACKS_STRUCT {
    void (*begin_file)(CodegenData_T* c);
    void (*finish_file)(CodegenData_T* c);
} BackendCallbacks_T;

typedef struct BACKEND_STRUCT {
    const char* name;

    Arch_T supported_architectures;
    Platform_T supported_platforms;
    bool supports_debug_info;

    const BackendCallbacks_T* callbacks;
} Backend_T;

extern const Backend_T COMPILER_BACKENDS[];

#define HEADER_DEFS_ONLY
    #include "backends/handlers.h"
#undef HEADER_DEFS_ONLY

const Backend_T* find_backend(const char* name);

bool backend_supports_target(const Backend_T* backend, Target_T* target);
const Backend_T* target_default_backend(Target_T* target);

#endif

