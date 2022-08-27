#include "dbg.h"
#include "debugger/breakpoint.h"
#include "io/log.h"
#include "io/io.h"
#include "globals.h"
#include "toolchain.h"
#include "platform/platform_bindings.h"
#include "register.h"

#include <stdarg.h>
#include <sys/ptrace.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define PROMPT_FMT COLOR_MAGENTA "%s" COLOR_RESET " [%s%d" COLOR_RESET "] >>"
#define ERROR_FMT  COLOR_BOLD_RED "[Error]" COLOR_RESET COLOR_RED " "
#define INFO_FMT   COLOR_BOLD_WHITE "[Info]" COLOR_RESET " "

static void handle_exit(const char* input);
static void handle_comp(const char* input);
static void handle_run(const char* input);
static void handle_help(const char* input);
static void handle_clear(const char* input);
static void handle_current(const char* input);
static void handle_load(const char* input);
static void handle_unload(const char* input);
static void handle_continue(const char* input);
static void handle_breakpoint(const char* input);
static void handle_register(const char* input);

static int 
    TRUE = true, 
    FALSE = false;

static const struct {
    char* cmd;
    void (*fn)(const char* input);
    int* enabled;
    const char* description;
} cmds[] = 
{
    {"help",    handle_help,    &TRUE,                   "Display this help text"},
    {"exit",    handle_exit,    &TRUE,                   "Exit the debugger"},
    {"clear",   handle_clear,   &TRUE,                   "Clear the screen"},
    {"comp",    handle_comp,    &TRUE,                   "Recompile the current source files"},
    {"run",     handle_run,     &TRUE,                   "Run the current executable"},
    {"current", handle_current, &TRUE,                   "Display the current debug target"},
    {"load",    handle_load,    &TRUE,                   "Load any executable for step-through debugging"},
    {"unload",  handle_unload,  &global.debugger.loaded, "Unload a loaded executable"},
    {"cont",   handle_continue, &global.debugger.loaded, "Continue executing a loaded executable"},
    {"brk", handle_breakpoint,  &global.debugger.loaded, "Set/Unset breakpoints in loaded executable"},
    {"register", handle_register, &global.debugger.loaded, "Read and modify registers"},
    {NULL, NULL, NULL, NULL}
};

static const char help_text_header[] = 
    COLOR_BOLD_MAGENTA " ** The CSpydr interactive debug shell **\n" COLOR_RESET
    "Current debug target: `%s` (compiled from `%s`).\n"
    "\n"
    COLOR_BOLD_WHITE "Available commands:\n" COLOR_RESET;
static const char help_cmd_fmt[] = "%s * " COLOR_BOLD_WHITE "%s" COLOR_RESET "%*s| %s\n";
static const char help_text_footer[] = "\n"
    COLOR_BOLD_WHITE "Prompt symbols:\n" COLOR_RESET
    "  \"" COLOR_MAGENTA "cspc" COLOR_RESET " [" COLOR_BOLD_GREEN "x" COLOR_RESET "] >>\"\n"
    COLOR_MAGENTA "    ^ " COLOR_BLUE "   ^~ exit code of the last command executed\n"
    COLOR_MAGENTA "    └~ path to the compiler executable\n" COLOR_RESET;
static const i32 max_help_cmd_len = 10;

static const char register_help_text[] = 
    COLOR_BOLD_MAGENTA " ** The CSpydr interactive debug shell **\n" COLOR_RESET
    "\n"
    "%s * " COLOR_BOLD_WHITE "register" COLOR_RESET " - Read and modify registers\n"
    COLOR_BLACK "(This command is only available if an executable is loaded.)\n" COLOR_RESET
    "\n"
    COLOR_BOLD_WHITE "Available subcommands:\n"
    "  help                    " COLOR_RESET " | Display this help text\n"
    COLOR_BOLD_WHITE "  dump                    " COLOR_RESET " | Print all register values\n"
    COLOR_BOLD_WHITE "  read " COLOR_RESET "[register]          | Read value from register\n"
    COLOR_BOLD_WHITE "  write " COLOR_RESET "[register] [value] | Write value to register\n"
    "\n";

static const char brk_help_text[] =
    COLOR_BOLD_MAGENTA " ** The CSpydr interactive debug shell **\n" COLOR_RESET
    "\n"
    "%s * " COLOR_BOLD_WHITE "brk" COLOR_RESET " - Set/Unset breakpoints in loaded executable\n"
    COLOR_BLACK "(This command is only available if an executable is loaded.)\n" COLOR_RESET
    "\n"
    COLOR_BOLD_WHITE "Available subcommands:\n"
    "  help    " COLOR_RESET "   | Display this help text\n"
    COLOR_BOLD_WHITE "  list " COLOR_RESET "      | List all breakpoints\n"
    COLOR_BOLD_WHITE "  add " COLOR_RESET "[addr] | Add a breakpoint at a given address\n"
    COLOR_BOLD_WHITE "  rm " COLOR_RESET "[addr]  | Remove a breakpoint from an address\n"
    "\n";

static inline bool prefix(const char *pre, const char *str)
{
    return strncmp(pre, str, strlen(pre)) == 0;
}

void debug_info(const char* fmt, ...)
{
    fprintf(stdout, INFO_FMT);

    va_list ap;
    va_start(ap, fmt);
    vfprintf(stdout, fmt, ap);
    va_end(ap);

    fprintf(stdout, "\n");
}

void debug_error(const char* fmt, ...)
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
        debug_info("Started debug session; for help, type `help`.");

    global.debugger.running = true;
    global.debugger.src_file = (char*) src;
    global.debugger.bin_file = (char*) bin;

    sprintf(global.debugger.prompt, PROMPT_FMT, global.exec_name, COLOR_BOLD_GREEN, 0);

    global.debugger.breakpoints = init_list();

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

    for(size_t i = 0; i < global.debugger.breakpoints->size; i++)
        free_breakpoint(global.debugger.breakpoints->items[i]);
    free_list(global.debugger.breakpoints);
}

static void handle_exit(const char* input)
{
    if(global.debugger.loaded)
    {
        if(question("An Executable is still loaded,\ninferior process `%d` will be killed.\n\nQuit anyway?", global.debugger.loaded))
            handle_unload(input);
        else
            return;
    }

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
    if(global.debugger.loaded)
    {
        debug_error("An executable is already loaded, please unload first. (pid %d)", global.debugger.loaded);
        return;
    }

    char* args = strdup(input);
    strtok(args, " ");

    char* exec = strtok(NULL, " ");
    List_T* exec_args = init_list();
    list_push(exec_args, exec);
    if(!exec) {
        debug_error("Command `lock` requires at least one argument, got 0.");
        goto fail;
    }

    for(char* arg; (arg = strtok(NULL, " ")); )
        list_push(exec_args, arg);
    list_push(exec_args, 0);

    // load the executable
    pid_t pid = fork();
    if(pid == 0)
    {
        // child process
        ptrace(PTRACE_TRACEME, 0, NULL, NULL);
        execv(exec, (char* const*) exec_args->items);
    }
    else if(pid > 0)
    {
        global.debugger.loaded = pid;
        if(!global.silent)
            debug_info("Loaded executable `%s` with pid `%d`.", exec, pid);
    }
    else 
    {
        debug_error("Failed to fork process, got pid `%d` in return.", pid);
        goto fail;
    }

fail:
    free_list(exec_args);
    free(args);
}

static void handle_unload(const char* input)
{
    if(!global.debugger.loaded)
    {
        debug_error("No executable is currently loaded.");
        return;
    }

    kill(global.debugger.loaded, SIGKILL);
    if(!global.silent)
        debug_info("Killed process with pid %d.", global.debugger.loaded);
    
    global.debugger.loaded = 0;
}

static void handle_continue(const char* input)
{
    if(!global.debugger.loaded)
    {
       debug_error("`cont` is only available if an executable is loaded.");
        return; 
    }

    ptrace(PTRACE_CONT, global.debugger.loaded, NULL, NULL);

    i32 wait_status,
        options = 0;
    waitpid(global.debugger.loaded, &wait_status, options);

    if(WIFEXITED(wait_status))
    {
        i32 exit_code = WEXITSTATUS(wait_status);
        debug_info("Process %d terminated with exit code %s%d" COLOR_RESET, global.debugger.loaded, exit_code ? COLOR_BOLD_RED : COLOR_BOLD_GREEN, exit_code);
        global.last_exit_code = exit_code;
        goto process_exited;
    }
    else if(WIFSIGNALED(wait_status))
    {
        debug_info("Process %d was killed by signal %s.", global.debugger.loaded, strsignal(WTERMSIG(wait_status)));
        global.last_exit_code = WTERMSIG(wait_status);
        goto process_exited;
    }
    else if(WIFSTOPPED(wait_status))
    {
        i32 stop_sig = WSTOPSIG(wait_status);
        switch(stop_sig)
        {
            case SIGTRAP:
                debug_info("Process %d hit breakpoint %d.", global.debugger.loaded, 0x0 /* TODO: determine breakpoint */);
                global.last_exit_code = 0;
                break;
            default:
                debug_info("Process %d was stopped by signal %s.", global.debugger.loaded, strsignal(WSTOPSIG(wait_status)));
                global.last_exit_code = WSTOPSIG(wait_status);
                goto process_exited;
        }
    }

    return;
process_exited:
    global.debugger.loaded = 0;
}

static void handle_breakpoint(const char* input)
{

    char* args = strdup(input);
    strtok(args, " "); // skip `brk`

    char* subcommand = strtok(NULL, " ");
    if(!subcommand)
    {
        debug_error("`brk` expects 1 argument, got 0");
        goto end;
    }

    if(strcmp(subcommand, "help") == 0)
        fprintf(OUTPUT_STREAM, brk_help_text, global.debugger.loaded ? COLOR_GREEN : COLOR_RED);
    else if(!global.debugger.loaded) 
    {
        debug_error("`brk` is only available if an executable is loaded.");
        return;
    }
    else if(strcmp(subcommand, "list") == 0)
    {
        debug_info("Currently set breakpoints:");
        if(global.debugger.breakpoints->size == 0)
        {
            fprintf(OUTPUT_STREAM, "  <none>\n\n");
            goto end;
        }

        for(size_t i = 0; i < global.debugger.breakpoints->size; i++)
        {
            Breakpoint_T* b = global.debugger.breakpoints->items[i];
            fprintf(OUTPUT_STREAM, "%s * " COLOR_RESET "0x%016lx\n", b->enabled ? COLOR_GREEN : COLOR_RED, b->addr);
        }
        fprintf(OUTPUT_STREAM, "\n");

        goto end;
    }
    else if(strcmp(subcommand, "rm") == 0)
    {
        char* addr_str = strtok(NULL, " ");
        if(!addr_str)
        {
            debug_error("`brk rm` expects address value");
            goto end;
        }

        if(strcmp(addr_str, "*") == 0)
        {
            for(size_t i = 0; i < global.debugger.breakpoints->size; i++)
            {
                Breakpoint_T* b = global.debugger.breakpoints->items[i];
                if(b->enabled)
                    breakpoint_disable(b);
            }
            goto end;
        }

        if(addr_str[0] != '0' || addr_str[1] != 'x')
        {
            debug_error("Address does not match `0x[0-9a-fA-F]+`");
            goto end;
        }

        intptr_t addr = strtol(addr_str, NULL, 16);
        disable_breakpoint_at_address(addr);
    }
    else if(strcmp(subcommand, "add") == 0)
    {
        char* addr_str = strtok(NULL, " ");
        if(!addr_str)
        {
            debug_error("`brk add` expects address value");
            goto end;
        }

        if(addr_str[0] != '0' || addr_str[1] != 'x')
        {
            debug_error("Address does not match `0x[0-9a-fA-F]+`");
            goto end;
        }

        intptr_t addr = strtol(addr_str, NULL, 16);
        set_breakpoint_at_address(addr);
    }

end:
    free(args);
}

static void handle_register(const char* input)
{
    char* args = strdup(input);
    strtok(args, " "); // skip `register`

    char* subcommand = strtok(NULL, " ");
    if(!subcommand)
    {
        debug_error("`register` expects one argument of [help, dump, read, write]");
        goto fail;
    }

    if(strcmp(subcommand, "help") == 0)
        fprintf(OUTPUT_STREAM, register_help_text, global.debugger.loaded ? COLOR_GREEN : COLOR_RED);
    else if(!global.debugger.loaded) 
    {
        debug_error("`register` is only available if an executable is loaded.");
        goto fail;
    }
    else if(strcmp(subcommand, "dump") == 0)
        dump_registers();
    else if(strcmp(subcommand, "read") == 0)
    {
        char* reg = strtok(NULL, " ");
        if(!reg) 
        {
            debug_error("`register read` expects register name");
            goto fail;
        }

        printf("%s %016lx\n", reg, get_register_value(global.debugger.loaded, get_register_from_name(reg)));
    }
    else if(strcmp(subcommand, "write") == 0)
    {
        char* reg = strtok(NULL, " ");
        if(!reg)
        {
            debug_error("`register write` expects register name");
            goto fail;
        }

        char* val_str = strtok(NULL, " ");
        if(!reg)
        {
            debug_error("`register write %s` expects value to write");
            goto fail;
        }

        if(val_str[0] != '0' || val_str[1] != 'x')
        {
            debug_error("Value does not match `0x[0-9a-fA-F]+`");
            goto fail;
        }

        u64 val = strtoul(val_str, NULL, 16);

        set_register_value(global.debugger.loaded, get_register_from_name(reg), val);
    }
    else 
    {
        debug_error("Unknown `register` subcommand `%s`, expect one of [help, dump, read, write].\n        Use `register help` to get help on this command.");
        goto fail;
    }

fail:
    free(args);    
}