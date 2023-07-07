#ifndef CSPYDR_INPUT_H
#define CSPYDR_INPUT_H

#include "list.h"
#include "file.h"
#include "context.h"

#include <stdbool.h>
#include <stdio.h>

File_T* read_file(const char* path);
void write_file(const char* path, char* buffer);
FILE *open_file(char *path);

bool file_exists(char* file);
bool file_is_readable(char* file);
bool file_is_writable(char* file);
bool file_is_executable(char* file);

void print_buffer(u8* buffer, size_t size);

bool question(const char* question, ...);
void clear_cache(Context_T*);
char* get_cache_dir(char* buffer);
char* get_cached_file_path(char* buffer, const char* filename, const char* fileextension);

#endif