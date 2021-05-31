#ifndef CSPYDR_FILE_H
#define CSPYDR_FILE_H

#include "../list.h"

typedef struct SRC_FILE_STRUCT 
{
    List_T* lines;
    unsigned int num_lines;

    const char* path;
} SrcFile_T;

SrcFile_T* init_srcfile(List_T* lines, const char* path);
void       free_srcfile(SrcFile_T* file);

char* get_line(SrcFile_T* file, unsigned int line);
char get_char(SrcFile_T* file, unsigned int line, unsigned int i);
unsigned int get_line_len(SrcFile_T* file, unsigned int line);

#endif