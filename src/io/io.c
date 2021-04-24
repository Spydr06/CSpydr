#include "io.h"

#include "file.h"
#include "log.h"

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

srcFile_T* readFile(const char* path)
{
    list_T* bufferList = initList(sizeof(char*));

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
        listPush(bufferList, strdup(line));
    }

    fclose(fp);
    if(line) 
    {
        free(line);
    }

    return initSrcFile(bufferList, path);
}

void writeFile(const char* path, char* buffer)
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