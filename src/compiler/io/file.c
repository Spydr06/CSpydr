#include "file.h"

#include <string.h>

SrcFile_T* init_srcfile(List_T* lines, const char* path)
{
    SrcFile_T* file = calloc(1, sizeof(SrcFile_T));

    file->lines = lines;
    file->path = path;
    file->num_lines = file->lines->size;
    file->short_path = NULL;

    return file;
}

void free_srcfile(SrcFile_T* file)
{
    for(i32 i = 0; i < file->num_lines; i++)
        free(file->lines->items[i]);

    if(file->short_path)
        free(file->short_path);

    free_list(file->lines);
    free(file);
}

char* get_line(SrcFile_T* file, u32 line)
{
    if(line >= file->num_lines)
        return NULL;
    
    return file->lines->items[line];
}

u32 get_line_len(SrcFile_T* file, u32 line)
{
    if(line >= file->num_lines)
        return 0;

    return strlen((char*) file->lines->items[line]);
}

char get_char(SrcFile_T* file, u32 line, u32 i)
{
    if(line >= file->num_lines || i >= get_line_len(file, line))
        return -1;

    return ((char*) file->lines->items[line])[i];
}