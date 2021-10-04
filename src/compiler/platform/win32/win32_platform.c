#include "win32_platform.h"

#if defined (_WIN32)

char* get_absolute_path(char* relative_path)
{
    //TODO: to be implemented
}

char* get_path_from_file(char* file_path)
{
    char dir[1024];
    _splitpath_s(path, NULL, 0, dir, sizeof(dir), NULL, 0, NULL, 0);
    return dir;
}
int make_dir(char* path)
{
    // TODO: to be implemented
    return 0;
}

#endif