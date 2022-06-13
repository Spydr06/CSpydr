#include "live.h"
#include "list.h"

#include <config.h>
#include <io/log.h>
#include <io/io.h>
#include <ast/ast.h>
#include <globals.h>
#include <parser/parser.h>
#include <mem/mem.h>

#include <stdlib.h>
#ifdef CSPYDR_LINUX
    #include <sys/inotify.h>
    #include <sys/wait.h>
    #include <unistd.h>
#endif

#define ERROR_MSG(str) COLOR_BOLD_RED "[Error] " COLOR_RESET COLOR_RED str COLOR_RESET

static List_T* lint_watched(const char* filepath)
{
    ASTProg_T ast = {};

    List_T* files = init_list();
    list_push(files, read_file(filepath));
    global.silent = true;

    pid_t pid = fork();
    switch(pid)
    {
        case -1:
            LOG_ERROR(ERROR_MSG("Could not fork process."));
            exit(1);
        case 0:
            parse(&ast, files, true);
            exit(0);
        default: {
            int exit_code;
            waitpid(pid, &exit_code, 0);
        }
    }

    return files;
}

static bool get_event(fd)
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

    int fd = inotify_init();
    if(fd < 0) {
        LOG_ERROR(ERROR_MSG("Could not initialize inotify."));
        exit(1);
    }

    bool relint = true;
    List_T* files = NULL;
    while(1) 
    {
        if(relint) 
        {
            relint = false;
            if(files != NULL)
            {
                for(size_t i = 0; i < files->size; i++)
                {
                    File_T* file = files->items[i];
                    inotify_rm_watch(fd, file->wd);
                    free_file(file);
                }
                free_list(files);
                mem_free();
            }

            files = lint_watched(filepath);
            for(size_t i = 0; i < files->size; i++)
            {
                File_T* file = files->items[i];
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