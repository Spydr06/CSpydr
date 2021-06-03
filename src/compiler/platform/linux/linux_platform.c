#include "linux_platform.h"

#if defined(__linux__) || defined (__linux)

char* get_absolute_path(char* relative_path) {
    return realpath(relative_path, NULL);
}

#endif