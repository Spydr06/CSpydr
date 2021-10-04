#ifndef CSPYDR_PLATFORM_BINDINGS_H
#define CSPYDR_PLATFORM_BINDINGS_H

#include <stdbool.h>

extern char* get_absolute_path(char* relative_path);
extern char* get_path_from_file(char* file_path);
extern char* get_home_directory();
extern bool make_dir(char* path);

#if defined(__linux__) || defined(__linux)
    #include "linux/linux_platform.h"
#elif defined(_WIN32)
    #include "win32/win32_platform.h"
#else
    #error CSpydr does not support your current platform
#endif

#endif