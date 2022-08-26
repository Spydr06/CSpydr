#include "dbg.h"
#include "io/log.h"
#include "globals.h"
#include "toolchain.h"
#include "platform/platform_bindings.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define PROMPT_FMT COLOR_MAGENTA "%s" COLOR_RESET " [%s%d" COLOR_RESET "] >>"
#define ERROR_FMT COLOR_BOLD_RED "[Error]" COLOR_RESET COLOR_RED " "

static void handle_exit(const char* input);
static void handle_comp(const char* input);
static void handle_run(const char* input);
static void handle_help(const char* input);
static void handle_clear(const char* input);
static void handle_current(const char* input);
static void handle_load(const char* input);

static bool 
    TRUE = true, 
    FALSE = false;

static const struct {
    char* cmd;
    void (*fn)(const char* input);
    bool* enabled;
    const char* description;
} cmds[] = 
{
    {"help",    handle_help,    &TRUE, "Display this help text"},
    {"exit",    handle_exit,    &TRUE, "Exit the debugger"},
    {"clear",   handle_clear,   &TRUE, "Clear the screen"},
    {"comp",    handle_comp,    &TRUE, "Recompile the current source files"},
    {"run",     handle_run,     &TRUE, "Run the current executable"},
    {"current", handle_current, &TRUE, "Display the current debug target"},
    {"load",    handle_load,    &TRUE, "Load any executable for step-through debugging"},
    {NULL, NULL, NULL, NULL}
};

static const char help_text_header[] = 
    COLOR_BOLD_MAGENTA " ** The CSpydr interactive debug shell **\n" COLOR_RESET
    "Current debug target: `%s` (compiled from `%s`).\n"
    "\n"
    COLOR_BOLD_WHITE "Available commands:\n" COLOR_RESET;
static const char help_cmd_fmt[] = "%s * " COLOR_BOLD_WHITE "%s" COLOR_RESET "%*s| %s\n";
static const char help_text_footer [] = "\n"
    COLOR_BOLD_WHITE "Prompt symbols:\n" COLOR_RESET
    "  \"" COLOR_MAGENTA "cspc" COLOR_RESET " [" COLOR_BOLD_GREEN "x" COLOR_RESET "] >>\"\n"
    COLOR_MAGENTA "    ^ " COLOR_BLUE "   ^~ exit code of the last command executed\n"
    COLOR_MAGENTA "    └~ path to the compiler executable\n" COLOR_RESET;
static const i32 max_help_cmd_len = 10;

static inline bool prefix(const char *pre, const char *str)
{
    return strncmp(pre, str, strlen(pre)) == 0;
}

static void debug_error(const char* fmt, ...)
{
    fprintf(stderr, ERROR_FMT);
    
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);

    fprintf(stderr, "\n");
}

void debug_repl(const char* src, const char* bin)
{
    if(!global.silent)
        LOG_INFO(COLOR_BOLD_WHITE "[Info]" COLOR_RESET " Started debug session; for help, type `help`.\n");

    global.debugger.running = true;
    global.debugger.src_file = (char*) src;
    global.debugger.bin_file = (char*) bin;

    sprintf(global.debugger.prompt, PROMPT_FMT, global.exec_name, COLOR_BOLD_GREEN, 0);

    while(global.debugger.running)
    {
        printf("%s ", global.debugger.prompt);
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
        debug_error("Unknown command `%s`.", input);
        global.last_exit_code = 1;
    skip:
        sprintf(global.debugger.prompt, PROMPT_FMT, global.exec_name, global.last_exit_code ? COLOR_BOLD_RED : COLOR_BOLD_GREEN, global.last_exit_code);
    }
}

static void handle_exit(const char* input)
{
    global.debugger.running = false;
}

static void handle_comp(const char* input)
{
    const char* args[] = {
        global.exec_name,
        "build",
        global.debugger.src_file,
        global.ct == CT_ASM ? "--asm" : "--transpile",
        NULL
    };

    global.last_exit_code = subprocess(args[0], (char* const*) args, false);
}

static void handle_run(const char* input)
{
    char local_executable[BUFSIZ] = {};
    sprintf(local_executable, "." DIRECTORY_DELIMS "%s", global.debugger.bin_file);
    const char* args[] = {
        local_executable,
        NULL        
    };

    global.last_exit_code = subprocess(args[0], (char* const*) args, false);
}

/*
    COLOR_GREEN " * " COLOR_RESET "help     | display this help text\n"
    COLOR_GREEN " * " COLOR_RESET "exit     | exit the debugger\n"
    COLOR_GREEN " * " COLOR_RESET "clear    | clear the screen\n"
    COLOR_GREEN " * " COLOR_RESET "run      | run the current executable\n"
    COLOR_GREEN " * " COLOR_RESET "comp     | recompile the current executable\n"
    COLOR_GREEN " * " COLOR_RESET "current  | display the current debug target\n"
    COLOR_GREEN " * "
*/

static void handle_help(const char* input)
{
    printf(help_text_header, global.debugger.bin_file, global.debugger.src_file);
    for(size_t i = 0; cmds[i].cmd; i++)
        printf(help_cmd_fmt, 
            *(cmds[i].enabled) ? COLOR_GREEN : COLOR_RED, 
            cmds[i].cmd, 
            max_help_cmd_len - (i32) strlen(cmds[i].cmd), 
            "", 
            cmds[i].description
        );
    printf(help_text_footer);
}

static void handle_clear(const char* input)
{   
    printf(CLEAR_CODE);
}

static void handle_current(const char* input)
{
    printf("`%s` (compiled from `%s`).\n", global.debugger.bin_file, global.debugger.src_file);
}

static void handle_load(const char* input)
{
    char* args = strdup(input);
    strtok(args, " ");

    char* exec = strtok(NULL, " ");
    if(!exec) {
        debug_error("Command `lock` requires at least one argument, got 0.");
        goto fail;
    }

    char* arg;
    while((arg = strtok(NULL, " ")) != NULL) {
        printf("args: %s\n", arg);
    }

    // TODO: implement

fail:
    free(args);
}