#ifndef CSPYDR_LINKER_H
#define CSPYDR_LINKER_H

#include <stdbool.h>
#include "config.h"

void link_obj(Context_T* context, const char* target, char* obj_file, bool silent, bool link_exec);

#endif
