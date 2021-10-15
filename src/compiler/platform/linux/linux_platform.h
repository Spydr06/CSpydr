#ifndef CSPYDR_LINUX_PLATFORM_H
#define CSPYDR_LINUX_PLATFORM_H

#if defined(__linux__) || defined (__linux)

#include <stdlib.h>
#include <linux/limits.h>
#include <stdbool.h>

// the default output file used for code generation
#define DEFAULT_OUTPUT_FILE "a.out"
// the characters between directories e.g.: /home/usr/...
#define DIRECTORY_DELIMS "/"

#define CACHE_DIR ".cache/cspydr"
#define STD_DIR "/usr/share/cspydr/std"

#define clrscr() printf("\e[1;1H\e[2J")

char* get_home_directory();

char* get_absolute_path(char* relative_path);
char* get_path_from_file(char* file_path);

bool make_dir(char* path);

int subprocess(const char* p_name, char* const* p_arg, bool print_exit_msg);


#endif
#endif