#include "file.h"

#include <string.h>

srcFile_T* initSrcFile(list_T* lines, const char* path)
{
    srcFile_T* file = calloc(1, sizeof(srcFile_T));

    file->lines = lines;
    file->path = path;
    file->numLines = file->lines->size;

    return file;
}

void freeSrcFile(srcFile_T* file)
{
    for(int i = 0; i < file->numLines; i++)
        free(file->lines->items[i]);

    freeList(file->lines);
    free(file);
}

char* getLine(srcFile_T* file, unsigned int line)
{
    if(line >= file->numLines)
        return NULL;
    
    return file->lines->items[line];
}

unsigned int getLineLength(srcFile_T* file, unsigned int line)
{
    if(line >= file->numLines)
        return 0;

    return strlen((char*) file->lines->items[line]);
}

char getChar(srcFile_T* file, unsigned int line, unsigned int i)
{
    if(line >= file->numLines || i >= getLineLength(file, line))
        return -1;

    return ((char*) file->lines->items[line])[i];
}