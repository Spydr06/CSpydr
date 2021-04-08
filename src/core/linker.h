#ifndef CSPYDR_LINKER_H
#define CSPYDR_LINKER_H

#include "list.h"

void linkObjectFiles(list_T* files, const char* executable, const char* stdPath);

#endif