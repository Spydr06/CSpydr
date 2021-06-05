#include <libgen.h>
#if defined(__linux__) || defined (__linux)

#include "linux_platform.h"

char* get_absolute_path(char* relative_path) 
{
    return realpath(relative_path, NULL);
}

char* get_path_from_file(char* file_path)
{
    return dirname(file_path);
}

#endif