#include "io.h"

#include "config.h"
#include "file.h"
#include "log.h"
#include "platform/platform_bindings.h"
#include "globals.h"

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

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

void clear_cache(void)
{
    char buffer[BUFSIZ] = {'\0'};
    get_cache_dir(buffer);

    if(!global.silent)
        LOG_OK_F(COLOR_BOLD_RED "  Deleting" COLOR_RESET "   %s\n", buffer);

    bool failure = remove_directory(buffer) == -1;
    if(failure)
    {
        if(errno == ENOENT) // no error gets thrown if the cache folder does not exist. Just return.
            return;
        LOG_ERROR_F(COLOR_BOLD_RED "[Error]" COLOR_RESET COLOR_RED " deleting `%s`: %s\n", buffer, strerror(errno));
        exit(1);
    }
}

char* get_cache_dir(char* buffer) 
{
    sprintf(buffer, "%s" DIRECTORY_DELIMS CACHE_DIR, get_home_directory());
    return buffer;
}

char* get_cached_file_path(char* buffer, const char* filename, const char* fileextension)
{
    get_cache_dir(buffer);
    strcat(buffer, DIRECTORY_DELIMS);
    strcat(buffer, filename);
    if(fileextension)
        strcat(buffer, fileextension);
}