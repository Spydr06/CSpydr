#include "live.h"
#include "ast/ast.h"
#include "error/panic.h"

#include <list.h>
#include <globals.h>
#include <io/io.h>
#include <io/log.h>
#include <mem/mem.h>
#include <passes.h>

#include <memory.h>
#include <stdlib.h>
#include <time.h> 
#ifdef CSPYDR_LINUX
    #include <sys/inotify.h>
    #include <sys/wait.h>
    #include <unistd.h>
#endif

#define ERROR_MSG(str) COLOR_BOLD_RED "[Error] " COLOR_RESET COLOR_RED str COLOR_RESET

static ASTProg_T AST = {0};

static inline void cleanup(void)
{
    cleanup_pass(&AST);
    mem_free();
}

static void free_ast(int fd) 
{
    if(AST.files != NULL)
    {
        for(size_t i = 0; i < AST.files->size; i++)
        {
            File_T* file = AST.files->items[i];
            if(fd) {
                inotify_rm_watch(fd, file->wd);
            }
        }
    }

    cleanup();
    globals_exit_hook();
}

static void sigint_handler(int dummy /* unused */) 
{
    if(question("\rDo you really want to quit?")) 
    {
        cleanup();
        exit(0);
    }
}

void lint_watched(const char* filepath, const char* std_path)
{
    init_globals();
    global.read_main_file_on_init = true;
    global.main_src_file = (char*) filepath;
    global.std_path = (char*) std_path;
    global.target = "/dev/null";

    memset(&AST, 0, sizeof(ASTProg_T));

    try(global.main_error_exception)
    {
        initialization_pass(&AST);
        lexer_pass(&AST);
        preprocessor_pass(&AST);
        parser_pass(&AST);
        validator_pass(&AST);
        typechecker_pass(&AST);
        if(global.emitted_errors)
            panic();
    }
    catch {
        get_panic_handler()();
        return;
    };

    time_t now;
    time(&now);
    struct tm* local = localtime(&now);

    LOG_OK_F(
        COLOR_GREEN "[" COLOR_BOLD_GREEN " Ok " COLOR_RESET COLOR_GREEN "] All good" COLOR_RESET 
        " (%lu file%s, at %02d:%02d:%02d)\n",
        AST.files->size, AST.files->size > 1 ? "s" : "", local->tm_hour, local->tm_min, local->tm_sec
    );
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

void live_session(const char* filepath, const char* std_path)
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

    bool first_time = true;
    bool relint = true;
    while(1) 
    {
        if(relint) 
        {
            relint = false;

            LOG_CLEAR();
            if(first_time)
                first_time = false;
            else
            {
                free_ast(fd);
                close(fd);
                fd = inotify_init();
            }

            lint_watched(filepath, std_path);
            for(size_t i = 0; i < AST.files->size; i++)
            {
                File_T* file = AST.files->items[i];
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