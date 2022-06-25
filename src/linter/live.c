#include "live.h"
#include "list.h"

#include <config.h>
#include <io/log.h>
#include <io/io.h>
#include <ast/ast.h>
#include <globals.h>
#include <parser/parser.h>
#include <mem/mem.h>
#include <error/panic.h>

#include <stdlib.h>
#ifdef CSPYDR_LINUX
    #include <sys/inotify.h>
    #include <sys/wait.h>
    #include <unistd.h>
#endif

#define ERROR_MSG(str) COLOR_BOLD_RED "[Error] " COLOR_RESET COLOR_RED str COLOR_RESET

static List_T* FILES = NULL;

static void free_files(int fd) 
{
    if(FILES != NULL)
    {
        for(size_t i = 0; i < FILES->size; i++)
        {
            File_T* file = FILES->items[i];
            if(fd) {
                inotify_rm_watch(fd, file->wd);
            }
            free_file(file);
        }
        free_list(FILES);
    }

    mem_free();
    globals_exit_hook();
}

static void sigint_handler(int dummy) 
{
    if(question(COLOR_BOLD_YELLOW "\rDo you really want to quit?")) 
        exit(dummy);
}

void lint_watched(const char* filepath)
{
    init_globals();

    FILES = init_list();
    list_push(FILES, read_file(filepath));

    ASTProg_T ast = {};
    try(global.main_error_exception)
    {
        global.silent = true;
        parse(&ast, FILES, true);
    }
    catch {
    };
}

static bool get_event(int fd)
{
    char buffer[BUFSIZ] = {};
    int len = read(fd, buffer, BUFSIZ);
    if(len < 0)
    {
        LOG_ERROR(ERROR_MSG("Could not read inotify."));
        return false;
    }

    return len > 0;
}

void live_session(const char* filepath)
{
#ifndef CSPYDR_LINUX
    LOG_ERROR(ERROR_MSG("live sessions not available on your current platform.\n"));
    exit(1);
#else

    signal(SIGINT, sigint_handler);

    int fd = inotify_init();
    if(fd < 0) {
        LOG_ERROR(ERROR_MSG("Could not initialize inotify."));
        exit(1);
    }

    bool relint = true;
    while(1) 
    {
        if(relint) 
        {
            relint = false;

            LOG_CLEAR();
            
            free_files(fd);
            close(fd);
            fd = inotify_init();

            lint_watched(filepath);
            for(size_t i = 0; i < FILES->size; i++)
            {
                File_T* file = FILES->items[i];
                file->wd = inotify_add_watch(fd, file->path, IN_MODIFY | IN_DELETE);
                if(file->wd == -1)
                    LOG_ERROR_F(ERROR_MSG("Could not add watch to %s.\n"), file->path);
                //else
                //    LOG_OK_F("Watching:: %s\n", file->path);
            }
        }

        if(get_event(fd))
            relint = true;
    }

    close(fd);

#endif
}