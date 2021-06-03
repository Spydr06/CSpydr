#ifndef CSPYDR_WIN32_PLATFORM_H
#define CSPYDR_WIN32_PLATFORM_H

#if defined (_WIN32)

#include <windows.h>

// the default output file used for code generation
#define DEFAULT_OUTPUT_FILE "a.exe"

// the characters between directories e.g.: C:\Users\...
#define DIRECTORY_DELIMS "\\" 

char* get_absolute_path(char* relative_path);

#endif
#endif