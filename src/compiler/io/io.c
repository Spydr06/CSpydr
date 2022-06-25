#include "io.h"

#include "config.h"
#include "file.h"
#include "log.h"

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

File_T* read_file(const char* path)
{
    List_T* buffer_list = init_list();

    FILE* fp;
    char* line = NULL;
    size_t len = 0;
    ssize_t read;

    fp = fopen(path, "r");
    if(fp == NULL) 
    {
        LOG_ERROR_F("Could not read file '%s'\n", path);
        exit(1);
    }

    while((read = getline(&line, &len, fp)) != -1) 
    {
        list_push(buffer_list, strdup(line));
    }

    fclose(fp);
    if(line) 
    {
        free(line);
    }

    return init_file(buffer_list, path);
}

FILE *open_file(char *path)
{
    if (!path || strcmp(path, "-") == 0)
        return stdout;

    FILE *out = fopen(path, "w");
    if (!out)
    {   
        LOG_ERROR_F("[Error] IO: could not open file: %s: %s\n", path, strerror(errno));
        exit(1);
    }
    return out;
}

void write_file(const char* path, char* buffer)
{
    FILE* fp;

    fp = fopen(path, "wb");
    if(fp == NULL)
    {
        LOG_ERROR_F("[ERROR] IO: could not open file '%s'\n", path);
        exit(1);
    }

    fprintf(fp, "%s", buffer);
    fclose(fp);
}

bool file_exists(char* file)
{
    return access(file, F_OK) != -1;
}

bool file_is_readable(char* file)
{
    return access(file, R_OK);
}

bool file_is_writable(char* file)
{
    return access(file, W_OK);
}

bool file_is_executable(char* file)
{
    return access(file, X_OK);
}

void print_buffer(u8* buffer, size_t size)
{
    fprintf(OUTPUT_STREAM, COLOR_BLUE "\t");

    if(size == 0 || !buffer)
    {
        fprintf(OUTPUT_STREAM, "00\n" COLOR_RESET);
        return;
    }

    for(size_t i = 0; i < size; i++)
    {
        fprintf(OUTPUT_STREAM, "%02x ", (u8) buffer[i]);
        if(i % 8 == 0 && i != 0)
            fprintf(OUTPUT_STREAM, "\n\t");
    }
    fprintf(OUTPUT_STREAM, "\n" COLOR_RESET);
}

bool question(const char* question)
{
    char answer = '\0';
    while(answer != 'y' && answer != 'Y' &&
        answer != 'n' && answer != 'N')
    {
        fprintf(OUTPUT_STREAM, "%s" COLOR_RESET " [" COLOR_BOLD_GREEN "y" COLOR_RESET "/" COLOR_BOLD_RED "n" COLOR_RESET "]: ", question);
        answer = getchar();
    }

    return answer == 'y' || answer == 'Y';
}