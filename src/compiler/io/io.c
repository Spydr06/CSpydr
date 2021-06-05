#include "io.h"

#include "file.h"
#include "log.h"

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

SrcFile_T* read_file(const char* path)
{
    List_T* buffer_list = init_list(sizeof(char*));

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

    return init_srcfile(buffer_list, path);
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

char* sh(const char* cmd) 
{
    char* output = (char*) calloc(1, sizeof(char));
    output[0] = '\0';

    FILE* fp;
    char path[1035];

    fp = popen(cmd, "r");

    if(fp == NULL) 
    {
        LOG_ERROR("Failed to run command\n");
        exit(1);
    }

    while(fgets(path, sizeof(path), fp) != NULL)
    {
        output = (char*) realloc(output, (strlen(output) + strlen(path) + 1) * sizeof(char));
        strcat(output, path);
    }

    pclose(fp);

    return output;
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