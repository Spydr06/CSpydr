#ifndef CSPYDR_WIN32_PLATFORM_H
#define CSPYDR_WIN32_PLATFORM_H

#if defined (_WIN32)

#include <windows.h>
#include <stdbool.h>

// the default output file used for code generation
#define DEFAULT_OUTPUT_FILE "a.exe"

// the characters between directories e.g.: C:\Users\...
#define DIRECTORY_DELIMS "\\" 

#define CACHE_DIR ".cache\\cspydr"
#define STD_DIR "%%appdata%%\\cspydr\\std"

char* get_home_directory();

char* get_absolute_path(char* relative_path);
char* get_path_from_file(char* file_path);

bool make_dir(char* path);

extern int subprocess(const char* p_name, const int p_argv, const char* p_argc[], void (*callback_fns[__EXIT_KIND_LEN])(void));

#endif
#endif