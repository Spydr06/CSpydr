#include "linker.h"
#include "../io.h"

#include <string.h>
#include <stdio.h>

#define CSP_LD "g++"
#define CSP_LD_FLAGS "" 

static char* generateObjFileStr(list_T* files)
{
    char* _string = calloc(1, sizeof(char));
    for(int i = 0; i < files->size; i++)
    {
        char* currentFile = files->items[i];
        _string = realloc(_string, (strlen(_string) + strlen(currentFile) + 2) * sizeof(char));
        strcat(_string, " ");
        strcat(_string, currentFile);
    }
    return _string;
}

void linkObjectFiles(list_T* files, const char* executable, const char *stdPath)
{
    const char* template = CSP_LD " " CSP_LD_FLAGS "-I%s -o %s %s";
    char* objFiles = generateObjFileStr(files); 
    char* command = calloc(strlen(template) + strlen(objFiles) + strlen(executable) + 1, sizeof(char));

    sprintf(command, template, stdPath, executable, objFiles);
    free(objFiles);

#if defined(__linux__) || defined(__linux)
    sh(command);
#elif defined(_WIN32) || defined(_WIN64)
    #error "CSpydr currently only supports linux. Windows support comes soon."
#else 
    #error "CSpydr does not support your current operating system."
#endif
}