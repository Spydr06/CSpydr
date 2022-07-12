#include "dbg.h"
#include "io/log.h"
#include "globals.h"
#include "toolchain.h"
#include "platform/platform_bindings.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define PROMPT_FMT COLOR_MAGENTA "%s" COLOR_RESET " [%s%d" COLOR_RESET "] >>"

static void handle_exit(const char* input);
static void handle_comp(const char* input);
static void handle_run(const char* input);
static void handle_help(const char* input);
static void handle_clear(const char* input);
static void handle_current(const char* input);

static const struct {
    char* cmd;
    void (*fn)(const char* input);
} cmds[] = 
{
    {"help", handle_help},
    {"exit", handle_exit},
    {"comp", handle_comp},
    {"run", handle_run},
    {"clear", handle_clear},
    {"current", handle_current},
    {NULL, NULL}
};

static const char help_text[] = 
    COLOR_BOLD_MAGENTA " ** The CSpydr interactive debug shell **\n" COLOR_RESET
    "Current debug target: `%s` (compiled from `%s`).\n"
    "\n"
    COLOR_BOLD_WHITE "Available commands:\n" COLOR_RESET
    COLOR_GREEN " * " COLOR_RESET "help     | display this help text\n"
    COLOR_GREEN " * " COLOR_RESET "exit     | exit the debugger\n"
    COLOR_GREEN " * " COLOR_RESET "clear    | clear the screen\n"
    COLOR_GREEN " * " COLOR_RESET "run      | run the current executable\n"
    COLOR_GREEN " * " COLOR_RESET "comp     | recompile the current executable\n"
    COLOR_GREEN " * " COLOR_RESET "current  | display the current debug target\n"
    "\n"
    COLOR_BOLD_WHITE "Prompt symbols:\n" COLOR_RESET
    "  \"" COLOR_MAGENTA "cspc" COLOR_RESET " [" COLOR_BOLD_GREEN "x" COLOR_RESET "] >>\"\n"
    COLOR_MAGENTA "    ^ " COLOR_BLUE "   ^~ exit code of the last command executed\n"
    COLOR_MAGENTA "    └~ path to the compiler executable\n" COLOR_RESET;

static char prompt[BUFSIZ];
static bool running;
static const char* src_file;
static const char* bin_file;

static inline bool prefix(const char *pre, const char *str)
{
    return strncmp(pre, str, strlen(pre)) == 0;
}

void debug_repl(const char* src, const char* bin)
{
    if(!global.silent)
        LOG_INFO(COLOR_BOLD_WHITE "[Info]" COLOR_RESET " Started debug session; for help, type `help`.\n");

    running = true;
    src_file = src;
    bin_file = bin;

    sprintf(prompt, PROMPT_FMT, global.exec_name, COLOR_BOLD_GREEN, 0);

    while(running)
    {
        printf("%s ", prompt);
        fflush(stdout);
        
        char input[BUFSIZ] = {'\0'};
        fgets(input, BUFSIZ - 1, stdin);
        input[strlen(input) - 1] = '\0';

        if(!strlen(input))
            continue;

        for(size_t i = 0; cmds[i].cmd; i++)
        {
            if(prefix(cmds[i].cmd, input))
            {
                cmds[i].fn(input);
                goto skip;
            }
        }
        LOG_ERROR_F(COLOR_BOLD_RED "[Error]" COLOR_RESET COLOR_RED " Unknown command `%s`.\n", input);
        global.last_exit_code = 1;
    skip:
        sprintf(prompt, PROMPT_FMT, global.exec_name, global.last_exit_code ? COLOR_BOLD_RED : COLOR_BOLD_GREEN, global.last_exit_code);
    }
}

static void handle_exit(const char* input)
{
    running = false;
}

static void handle_comp(const char* input)
{
    const char* args[] = {
        global.exec_name,
        "build",
        src_file,
        global.ct == CT_ASM ? "--asm" : "--transpile",
        NULL
    };

    global.last_exit_code = subprocess(args[0], (char* const*) args, false);
}

static void handle_run(const char* input)
{
    char local_executable[BUFSIZ] = {};
    sprintf(local_executable, "." DIRECTORY_DELIMS "%s", bin_file);
    const char* args[] = {
        local_executable,
        NULL        
    };

    global.last_exit_code = subprocess(args[0], (char* const*) args, false);
}

static void handle_help(const char* input)
{
    printf(help_text, bin_file, src_file);
}

static void handle_clear(const char* input)
{   
    printf(CLEAR_CODE);
}

static void handle_current(const char* input)
{
    printf("`%s` (compiled from `%s`).\n", bin_file, src_file);
}