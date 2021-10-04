#include <asm-generic/errno-base.h>
#if defined(__linux__) || defined (__linux)

#include "linux_platform.h"

#include <libgen.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/types.h>

char* get_absolute_path(char* relative_path) 
{
    return realpath(relative_path, NULL);
}

char* get_path_from_file(char* file_path)
{
    return dirname(file_path);
}

char* get_home_directory()
{
    return getenv("HOME");
}

bool make_dir(char* path)
{
    int error = mkdir(path, 0777);
    if(!error || (error && errno == EEXIST))
        return 0;
    return 1;
}

#endif