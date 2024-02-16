#ifndef CSPYDR_CODEGEN_BACKEND_H
#define CSPYDR_CODEGEN_BACKEND_H

#include "context.h"
#include <stdbool.h>

typedef struct TARGET_STRUCT Target_T;

#define HEADER_DEFS_ONLY
#include "backends/handlers.h"

typedef struct BACKEND_STRUCT {
    const char* name;

    Arch_T supported_architectures;
    Platform_T supported_platforms;
} Backend_T;

extern const Backend_T COMPILER_BACKENDS[];

const Backend_T* find_backend(const char* name);

bool backend_supports_target(const Backend_T* backend, Target_T* target);
const Backend_T* target_default_backend(Target_T* target);

#endif

