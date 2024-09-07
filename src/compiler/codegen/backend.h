#ifndef CSPYDR_CODEGEN_BACKEND_H
#define CSPYDR_CODEGEN_BACKEND_H

#include "context.h"
#include "ir/ir.h"
#include <stdbool.h>

#define BACKEND_CALLBACK(backend, callback) backend##_##callback
#define BACKEND_CALLBACKS_IMPL_EXT(backend)                                  \
    const BackendCallbacks_T backend##_CALLBACKS = ((BackendCallbacks_T) {   \
        .begin_file = backend##_begin_file,                                  \
        .finish_file = backend##_finish_file,                                \
        .generate_ir = backend##_generate_ir,                                \
        .compile = backend##_compile,                                        \
    })

#define BACKEND_CALLBACKS_IMPL(backend) BACKEND_CALLBACKS_IMPL_EXT(backend)

typedef struct TARGET_STRUCT Target_T;
typedef struct CODEGEN_DATA_STRUCT CodegenData_T;

typedef struct BACKEND_CALLBACKS_STRUCT {
    void (*const begin_file)(CodegenData_T* c);
    void (*const finish_file)(CodegenData_T* c);
    void (*const generate_ir)(CodegenData_T* c, IR_T*);
    void (*const compile)(CodegenData_T* c, const char* input_path, const char* output_path);
} BackendCallbacks_T;

typedef enum OUTPUT_FORMAT_ENUM : u8 {
    OUTPUT_FORMAT_TEXT,
    OUTPUT_FORMAT_RAW,
} OutputFormat_T;

typedef struct BACKEND_STRUCT {
    const char* name;

    Arch_T supported_architectures;
    Platform_T supported_platforms;

    OutputFormat_T output_format;
    bool supports_debug_info;

    const BackendCallbacks_T* callbacks;
    const char* fileext;
} Backend_T;

extern const Backend_T COMPILER_BACKENDS[];

#define HEADER_DEFS_ONLY
    #include "backends/handlers.h"
#undef HEADER_DEFS_ONLY

const Backend_T* find_backend(const char* name);

bool backend_supports_target(const Backend_T* backend, Target_T* target);
const Backend_T* target_default_backend(Target_T* target);

#endif

