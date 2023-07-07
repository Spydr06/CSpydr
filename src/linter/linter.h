#ifndef CSPL_LINTER_H
#define CSPL_LINTER_H

#include <util.h>
#include "context.h"

i32 lint(Context_T* context, char* src_file, char* std_path);

#endif