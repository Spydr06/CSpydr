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
bool make_dir(char* path)
{
    // TODO: to be implemented
    return 0;
}

extern int subprocess(const char* p_name, const char* arg, void (*callback_fns[__EXIT_KIND_LEN])(void))
{
    #error "Implement this"
}

#endif