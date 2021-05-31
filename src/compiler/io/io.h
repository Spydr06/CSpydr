#ifndef CSPYDR_INPUT_H
#define CSPYDR_INPUT_H

#include "../list.h"
#include "file.h"

SrcFile_T* read_file(const char* path);
void write_file(const char* path, char* buffer);
char* sh(const char* cmd);

#endif