#include "file.h"

#include <string.h>

File_T* init_file(List_T* lines, const char* path)
{
    File_T* file = malloc(sizeof(File_T));

    file->lines = lines;
    file->path = (char*) path;
    file->num_lines = file->lines->size;
    file->short_path = NULL;
    file->file_no = 0;

    return file;
}

void free_file(File_T* file)
{
    for(size_t i = 0; i < file->num_lines; i++)
        free(file->lines->items[i]);

    if(file->short_path)
        free(file->short_path);

    free_list(file->lines);
    free(file);
}

char* get_line(File_T* file, u32 line)
{
    if(line >= file->num_lines)
        return NULL;
    
    return file->lines->items[line];
}

u32 get_line_len(File_T* file, u32 line)
{
    if(line >= file->num_lines)
        return 0;

    return strlen((char*) file->lines->items[line]);
}

char get_char(File_T* file, u32 line, u32 i)
{
    if(line >= file->num_lines || i >= get_line_len(file, line))
        return -1;

    return ((char*) file->lines->items[line])[i];
}