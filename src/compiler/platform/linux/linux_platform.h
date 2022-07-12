#ifndef CSPYDR_LINUX_PLATFORM_H
#define CSPYDR_LINUX_PLATFORM_H

#if defined(__linux__) || defined (__linux)

#define _XOPEN_SOURCE 500
#include <stdlib.h>
#include <linux/limits.h>
#undef  _XOPEN_SOURCE
#include <stdbool.h>
#include "../../util.h"

// the default output file used for code generation
#define DEFAULT_OUTPUT_FILE "a.out"
// the characters between directories e.g.: /home/usr/...
#define DIRECTORY_DELIMS "/"

#define CACHE_DIR ".cache/cspydr"

#define clrscr() printf("\e[1;1H\e[2J")

char* get_home_directory();

char* get_absolute_path(char* relative_path);
char* get_path_from_file(char* file_path);

bool make_dir(char* path);

i32 remove_directory(const char* dirname);

i32 subprocess(const char* p_name, char* const* p_arg, bool pri32_exit_msg);

#endif
#endif