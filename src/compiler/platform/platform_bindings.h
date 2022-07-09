#ifndef CSPYDR_PLATFORM_BINDINGS_H
#define CSPYDR_PLATFORM_BINDINGS_H

#include <stdbool.h>
#include "util.h"

char* get_absolute_path(char* relative_path);
char* get_path_from_file(char* file_path);
char* get_home_directory();
bool make_dir(char* path);
i32 subprocess(const char* p_name, char* const* p_arg, bool pri32_exit_msg);
i32 remove_directory(const char* dirname);

#if defined(__linux__) || defined(__linux)
    #include "linux/linux_platform.h"
#elif defined(_WIN32)
    #include "win32/win32_platform.h"
#else
    #error CSpydr does not support your current platform
#endif

#endif