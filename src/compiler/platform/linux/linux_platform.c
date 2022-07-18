#if defined(__linux__) || defined (__linux)

#include "linux_platform.h"
#include "io/log.h"

#define __USE_XOPEN2K8 1
#include <string.h>
#undef  __USE_XOPEN2K8

#define _XOPEN_SOURCE 500

#include <libgen.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <ftw.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#undef  _XOPEN_SOURCE

char* get_absolute_path(char* relative_path) 
{
    return realpath(relative_path, NULL);
}

char* get_path_from_file(char* file_path)
{
    return dirname(file_path);
}

char* get_home_directory()
{
    return getenv("HOME");
}

bool make_dir(char* path)
{
    i32 error = mkdir(path, 0777);
    if(!error || (error && errno == EEXIST))
        return 0;
    return 1;
}

i32 subprocess(const char* p_name, char* const* p_arg, bool print_exit_msg)
{
    pid_t pid = fork();

    if(pid < 0)
    {
        LOG_ERROR_F("could not create subprocess for %s. <error code %d>\n", p_name, pid);
        return -1;
    }

    if(pid == 0 && execvp(p_name, p_arg) == -1)
    {
        LOG_ERROR_F("error executing %s: %s\n", p_name, strerror(errno));
        exit(-1);
    }

    i32 pid_status;
    if(waitpid(pid, &pid_status, 0) == -1)
    {
        LOG_ERROR_F("error getting status of child process %d\n", pid);
        return -1;
    }

    if(print_exit_msg)
    {
        if(WIFEXITED(pid_status))
        {
            i32 exit_code = WEXITSTATUS(pid_status);
            LOG_INFO_F(COLOR_RESET "[%s terminated with exit code %s%d" COLOR_RESET "]\n", p_name, exit_code ? COLOR_BOLD_RED : COLOR_BOLD_GREEN, exit_code);
        }
        else if(WIFSIGNALED(pid_status))
            LOG_INFO_F(COLOR_BOLD_RED "[%s killed by signal %s (%d)]\n", p_name, strsignal(WTERMSIG(pid_status)), WTERMSIG(pid_status));
        else if(WIFSTOPPED(pid_status))
            LOG_INFO_F(COLOR_BOLD_RED "[%s stopped by signal %s (%d)\n", p_name, strsignal(WSTOPSIG(pid_status)), WSTOPSIG(pid_status));
#ifdef WCOREDUMP
        else if(WCOREDUMP(pid_status))
            LOG_INFO_F(COLOR_YELLOW "[%s core dumped]\n", p_name);
#endif
    }

    return WEXITSTATUS(pid_status);
}

static i32 unlink_cb(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf)
{
    i32 rv = remove(fpath);

    if (rv)
        perror(fpath);

    return rv;
}

i32 remove_directory(const char* dirname)
{
    return nftw(dirname, unlink_cb, 64, FTW_DEPTH | FTW_PHYS);
}

#endif