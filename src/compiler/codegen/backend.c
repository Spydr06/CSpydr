#include "backend.h"
#include "context.h"

#include <string.h>
#include <stdlib.h>

const Backend_T COMPILER_BACKENDS[] = {
    #undef HEADER_DEFS_ONLY
    #include "backends/handlers.h"
    {NULL}
};

const Backend_T* find_backend(const char* name)
{
    for(const Backend_T* backend = COMPILER_BACKENDS; backend->name; backend++)
        if(strcmp(backend->name, name) == 0)
            return backend;

    return NULL;
}

bool backend_supports_target(const Backend_T* backend, Target_T* target)
{
    return (backend->supported_platforms & target->platform) && (backend->supported_architectures & target->arch);
}

const Backend_T* target_default_backend(Target_T* target)
{
    for(const Backend_T* backend = COMPILER_BACKENDS; backend->name; backend++)
        if(backend_supports_target(backend, target))
            return backend;
    return NULL;
}


