#include "input.h"

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>


char* readFile(const char* path)
{
    FILE* fp;
    char* line = NULL;
    size_t len = 0;
    ssize_t read;

    fp = fopen(path, "r");
    if(fp == NULL) 
    {
        fprintf(stderr, "[IO] Could not read file '%s'\n", path);
        exit(1);
    }

    char* buffer = (char*) calloc(1, sizeof(char));
    buffer[0] = '\0';

    while((read = getline(&line, &len, fp)) != -1) 
    {
        buffer = (char*) realloc(buffer, (strlen(buffer) + strlen(line) + 1) * sizeof(char));
        strcat(buffer, line);
    }

    fclose(fp);
    if(line) 
    {
        free(line);
    }

    return buffer;
}

void writeFile(const char* path, char* buffer)
{
    FILE* fp;

    fp = fopen(path, "wb");
    if(fp == NULL)
    {
        fprintf(stderr, "[ERROR] IO: could not open file '%s'\n", path);
        exit(1);
    }

    fprintf(fp, "%s", buffer);
    fclose(fp);
}