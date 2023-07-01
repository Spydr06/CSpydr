#ifndef CSPYDR_LINKER_H
#define CSPYDR_LINKER_H

#include <stdbool.h>

void link_obj(const char* target, char* obj_file, bool silent, bool link_exec);

#endif
