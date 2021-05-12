#ifndef CSPYDR_INPUT_H
#define CSPYDR_INPUT_H

#include "../list.h"
#include "file.h"

srcFile_T* readFile(const char* path);
void writeFile(const char* path, char* buffer);
char* sh(const char* cmd);

#endif