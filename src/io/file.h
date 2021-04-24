#ifndef CSPYDR_FILE_H
#define CSPYDR_FILE_H

#include "../list.h"

typedef struct SRC_FILE_STRUCT 
{
    list_T* lines;
    unsigned int numLines;

    const char* path;
} srcFile_T;

srcFile_T* initSrcFile(list_T* lines, const char* path);
void freeSrcFile(srcFile_T* file);

char* getLine(srcFile_T* file, unsigned int line);
char getChar(srcFile_T* file, unsigned int line, unsigned int i);
unsigned int getLineLength(srcFile_T* file, unsigned int line);

#endif