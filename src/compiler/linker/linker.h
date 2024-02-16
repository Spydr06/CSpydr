#ifndef CSPYDR_LINKER_H
#define CSPYDR_LINKER_H

#include "context.h"

i32 linker_pass(Context_T* context, const char* target_filepath, const char* object_filepath);

#endif

