#ifndef CSPYDR_WIN32_PLATFORM_H
#define CSPYDR_WIN32_PLATFORM_H

#if defined (_WIN32)

#include <windows.h>

// the default output file used for code generation
#define DEFAULT_OUTPUT_FILE "a.exe"

// the characters between directories e.g.: C:\Users\...
#define DIRECTORY_DELIMS "\\" 

#define CACHE_DIR ".cache\\cspydr"
#define STD_DIR "%%appdata%%\\cspydr\\std"

char* get_home_directory();

char* get_absolute_path(char* relative_path);
char* get_path_from_file(char* file_path);

#endif
#endif