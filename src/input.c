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

char* sh(const char* cmd) 
{
    char* output = (char*) calloc(1, sizeof(char));
    output[0] = '\0';

    FILE* fp;
    char path[1035];

    fp = popen(cmd, "r");

    if(fp == NULL) 
    {
        printf("Failed to run command\n");
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