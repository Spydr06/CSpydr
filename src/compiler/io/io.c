#include "io.h"

#include "config.h"
#include "file.h"
#include "log.h"
#include "platform/linux/linux_platform.h"
#include "platform/platform_bindings.h"

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <libgen.h>
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

bool file_exists(const char* file)
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

bool question(const char* question, ...)
{
    char answer = '\0';
    while(answer != 'y' && answer != 'Y' &&
        answer != 'n' && answer != 'N')
    {
        va_list ap;
        va_start(ap, question);
        vfprintf(OUTPUT_STREAM, question, ap);
        va_end(ap);
        fprintf(OUTPUT_STREAM, COLOR_RESET " [" COLOR_BOLD_GREEN "y" COLOR_RESET "/" COLOR_BOLD_RED "n" COLOR_RESET "]: ");
        answer = getchar();
    }

    return answer == 'y' || answer == 'Y';
}

void clear_cache(Context_T* context)
{
    char buffer[BUFSIZ] = {'\0'};
    get_cache_dir(buffer);

    if(!context->flags.silent)
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

    strcat(buffer, basename((char*) filename));
    if(fileextension)
        strcat(buffer, fileextension);
    return buffer;
}

int finsert (FILE* file, const char *buffer) 
{
    long int insert_pos = ftell(file);
    if (insert_pos < 0) return insert_pos;

    // Grow from the bottom
    int seek_ret = fseek(file, 0, SEEK_END);
    if (seek_ret) return seek_ret;
    long int total_left_to_move = ftell(file);
    if (total_left_to_move < 0) return total_left_to_move;

    char move_buffer[1024];
    unsigned long int amount_to_grow = strlen(buffer);
    if (amount_to_grow >= sizeof(move_buffer)) return -1;

    total_left_to_move -= insert_pos;

    for(;;) {
        u16 amount_to_move = sizeof(move_buffer);
        if (total_left_to_move < amount_to_move) amount_to_move = total_left_to_move;

        long int read_pos = insert_pos + total_left_to_move - amount_to_move;

        seek_ret = fseek(file, read_pos, SEEK_SET);
        if (seek_ret) return seek_ret;
        fread(move_buffer, amount_to_move, 1, file);
        if (ferror(file)) return ferror(file);

        seek_ret = fseek(file, read_pos + amount_to_grow, SEEK_SET);
        if (seek_ret) return seek_ret;
        fwrite(move_buffer, amount_to_move, 1, file);
        if (ferror(file)) return ferror(file);

        total_left_to_move -= amount_to_move;

        if (!total_left_to_move) break;

    }

    seek_ret = fseek(file, insert_pos, SEEK_SET);
    if (seek_ret) return seek_ret;
    fwrite(buffer, amount_to_grow, 1, file);
    if (ferror(file)) return ferror(file);

    return 0;
}
