#include "file.h"

#include <string.h>

SrcFile_T* init_srcfile(List_T* lines, const char* path)
{
    SrcFile_T* file = calloc(1, sizeof(SrcFile_T));

    file->lines = lines;
    file->path = path;
    file->num_lines = file->lines->size;

    return file;
}

void free_srcfile(SrcFile_T* file)
{
    for(int i = 0; i < file->num_lines; i++)
        free(file->lines->items[i]);

    free_list(file->lines);
    free(file);
}

char* get_line(SrcFile_T* file, unsigned int line)
{
    if(line >= file->num_lines)
        return NULL;
    
    return file->lines->items[line];
}

unsigned int get_line_len(SrcFile_T* file, unsigned int line)
{
    if(line >= file->num_lines)
        return 0;

    return strlen((char*) file->lines->items[line]);
}

char get_char(SrcFile_T* file, unsigned int line, unsigned int i)
{
    if(line >= file->num_lines || i >= get_line_len(file, line))
        return -1;

    return ((char*) file->lines->items[line])[i];
}