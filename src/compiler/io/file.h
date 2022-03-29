#ifndef CSPYDR_FILE_H
#define CSPYDR_FILE_H

#include "list.h"
#include "util.h"

typedef struct SRC_FILE_STRUCT 
{
    List_T* lines;
    u32 num_lines;
    u32 file_no;

    const char* path;
    char* short_path;
} File_T;

File_T* init_file(List_T* lines, const char* path);
void free_file(File_T* file);

char* get_line(File_T* file, u32 line);
char get_char(File_T* file, u32 line, u32 i);
u32 get_line_len(File_T* file, u32 line);

#endif