#ifndef CSPYDR_API_H
#define CSPYDR_API_H

#include <stdbool.h>

#define __CSPYDR_INTERNAL_USE
#include "include/cspydr.h"
#include "context.h"

typedef struct CSPYDR_COMPILER_STRUCT {
    bool initialized;
    CompilerStatus_T status;
    Context_T context;
} Compiler_T;

#endif