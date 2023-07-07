#include "live.h"
#include "ast/ast.h"
#include "config.h"
#include "context.h"
#include "error/panic.h"

#include <list.h>
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
static bool prompt_on_quit = true;

static inline void cleanup(Context_T* context)
{
    cleanup_pass(context, &AST);
    mem_free();
}

static void free_ast(Context_T* context, int fd) 
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

    cleanup(context);
}

static Context_T* __sighandler_context_ptr = 0;
static void sigint_handler(int dummy /* unused */) 
{
    if(prompt_on_quit && !question("\rDo you really want to quit?")) 
        return;

    cleanup(__sighandler_context_ptr);
    exit(0);
}

void lint_watched(Context_T* context, const char* filepath, const char* std_path)
{
    init_context(context);
    context->flags.read_main_file_on_init = true;
    context->paths.main_src_file = (char*) filepath;
    context->paths.std_path = (char*) std_path;
    context->paths.target = "/dev/null";

    memset(&AST, 0, sizeof(ASTProg_T));

    try(context->main_error_exception)
    {
        initialization_pass(context, &AST);
        lexer_pass(context, &AST);
        preprocessor_pass(context, &AST);
        parser_pass(context, &AST);
        validator_pass(context, &AST);
        typechecker_pass(context, &AST);
        if(context->emitted_errors)
            panic(context);
    }
    catch {
        get_panic_handler()(context);
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

void live_session(Context_T* context, const char* filepath, const char* std_path, bool _prompt_on_quit)
{
    prompt_on_quit = _prompt_on_quit;
#ifndef CSPYDR_LINUX
    LOG_ERROR(ERROR_MSG("live sessions not available on your current platform.\n"));
    exit(1);
#else

    __sighandler_context_ptr = context;
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
                free_ast(context, fd);
                close(fd);
                fd = inotify_init();
            }

            lint_watched(context, filepath, std_path);
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