#ifndef CSPYDR_LINUX_PLATFORM_H
#define CSPYDR_LINUX_PLATFORM_H

#if defined(__linux__) || defined (__linux)

#include <stdlib.h>
#include <linux/limits.h>
// the default output file used for code generation
#define DEFAULT_OUTPUT_FILE "a.out"
// the characters between directories e.g.: /home/usr/...
#define DIRECTORY_DELIMS "/"

char* get_absolute_path(char* relative_path);

#endif
#endif