#ifndef CSPYDR_FILE_H
#define CSPYDR_FILE_H

#include "../list.h"
#include "../util.h"

typedef struct SRC_FILE_STRUCT 
{
    List_T* lines;
    u32 num_lines;

    const char* path;
    char* short_path;
} SrcFile_T;

SrcFile_T* init_srcfile(List_T* lines, const char* path);
void       free_srcfile(SrcFile_T* file);

char* get_line(SrcFile_T* file, u32 line);
char get_char(SrcFile_T* file, u32 line, u32 i);
u32 get_line_len(SrcFile_T* file, u32 line);

#endif