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
#include <errno.h>

#define PROMPT_FMT COLOR_MAGENTA "%s" COLOR_RESET " [%s%d" COLOR_RESET "] >>"
#define ERROR_FMT  COLOR_BOLD_RED "[Error]" COLOR_RESET COLOR_RED " "
#define INFO_FMT   COLOR_BOLD_WHITE "[Info]" COLOR_RESET " "

#define CSPYDR_DEBUGGER_HEADER COLOR_BOLD_MAGENTA " ** The CSpydr interactive debug shell **\n" COLOR_RESET

static void handle_exit(const char* input);
static void handle_comp(const char* input);
static void handle_help(const char* input);
static void handle_clear(const char* input);
static void handle_current(const char* input);
static void handle_load(const char* input);
static void handle_unload(const char* input);
static void handle_continue(const char* input);
static void handle_breakpoint(const char* input);
static void handle_register(const char* input);
static void handle_memory(const char* input);
static void handle_sh(const char* input);

static int ALWAYS_TRUE = true;

static const struct {
    char* cmd;
    void (*fn)(const char* input);
    int* enabled;
    bool enabled_when_true;
    const char* description;
} cmds[] = 
{
    { "help",     handle_help,       &ALWAYS_TRUE,              true,  "Display this help text"                         },
    { "exit",     handle_exit,       &ALWAYS_TRUE,              true,  "Exit the debugger"                              },
    { "clear",    handle_clear,      &ALWAYS_TRUE,              true,  "Clear the screen"                               },
    { "comp",     handle_comp,       &ALWAYS_TRUE,              true,  "Recompile the current source files"             },
    { "current",  handle_current,    &ALWAYS_TRUE,              true,  "Display the current debug target"               },
    { "sh",       handle_sh,         &ALWAYS_TRUE,              true,  "Run an external shell command"                  },
    { "load",     handle_load,       &global.debugger.loaded,   false, "Load any executable for step-through debugging" },
    { "unload",   handle_unload,     &global.debugger.loaded,   true,  "Unload a loaded executable"                     },
    { "cont",     handle_continue,   &global.debugger.loaded,   true,  "Continue executing a loaded executable"         },
    { "brk",      handle_breakpoint, &global.debugger.loaded,   true,  "Set/Unset breakpoints in loaded executable"     },
    { "register", handle_register,   &global.debugger.loaded,   true,  "Read and modify registers"                      },
    { "memory",   handle_memory,     &global.debugger.loaded,   true,  "Read and modify program memory"                 },
    { NULL,       NULL,              NULL,                      false, NULL                                             }
};

static const char help_text_header[] = 
    CSPYDR_DEBUGGER_HEADER
    "Current debug target: `%s` (compiled from `%s`).\n"
    "\n"
    COLOR_BOLD_WHITE "Available commands:\n" COLOR_RESET;
static const char help_cmd_fmt[] = "%s * " COLOR_BOLD_WHITE "%s" COLOR_RESET "%*s| %s\n";
static const char help_text_footer[] = "\n"
    COLOR_BOLD_WHITE "Tipp:" COLOR_RESET "\n"
    "  Use `<command> help` to get more information on a specific command.\n"
    "\n"
    COLOR_BOLD_WHITE "Prompt symbols:\n" COLOR_RESET
    "  \"" COLOR_MAGENTA "cspc" COLOR_RESET " [" COLOR_BOLD_GREEN "x" COLOR_RESET "] >>\"\n"
    COLOR_MAGENTA "    ^ " COLOR_BLUE "   ^~ exit code of the last command executed\n"
    COLOR_MAGENTA "    └~ path to the compiler executable\n" COLOR_RESET;
static const i32 max_help_cmd_len = 10;

static const char register_help_text[] = 
    CSPYDR_DEBUGGER_HEADER
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
    CSPYDR_DEBUGGER_HEADER
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

static const char memory_help_text[] =
    CSPYDR_DEBUGGER_HEADER
    "\n"
    "%s * " COLOR_BOLD_WHITE "memory" COLOR_RESET " - Read and modify program memory\n"
    COLOR_BLACK "(This command is only available if an executable is loaded.)\n" COLOR_RESET
    "\n"
    COLOR_BOLD_WHITE "Available subcommands:\n"
                     "  help                   " COLOR_RESET " | Display this help text\n"
    COLOR_BOLD_WHITE "  read " COLOR_RESET "[address]          | Read value at memory at address\n"
    COLOR_BOLD_WHITE "  write " COLOR_RESET "[address] [value] | Write value to memory at address\n"
    "\n";

static const char load_help_text[] =
    CSPYDR_DEBUGGER_HEADER
    "\n"
    "%s * " COLOR_BOLD_WHITE "load" COLOR_RESET " - Load any executable for step-through debugging\n"
    COLOR_BLACK "(This command is only available if no executable is already loaded.)\n" COLOR_RESET
    "\n"
    COLOR_BOLD_WHITE "Available subcommands:\n"
                     "  help        " COLOR_RESET " | Display this help text\n"
    COLOR_BOLD_WHITE "  [executable]" COLOR_RESET " | Load this executable into memory for debugging\n"
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
    bool was_loaded = global.debugger.loaded;
    if(global.debugger.loaded)
    {
        if(question("An Executable is still loaded,\ninferior process `%d` will be killed.\n\nContinue anyway?", global.debugger.loaded))
            handle_unload(input);
        else
            return;
    }

    fprintf(OUTPUT_STREAM, "\n");

    const char* args[] = {
        global.exec_name,
        "build",
        global.debugger.src_file,
        global.ct == CT_ASM ? "--asm" : "--transpile",
        NULL
    };

    global.last_exit_code = subprocess(args[0], (char* const*) args, false);

    fprintf(OUTPUT_STREAM, "\n");

    if(was_loaded)
        handle_load(global.debugger.loaded_cmd);
}

static u64 hex_value_arg(const char* cmd_name)
{
    errno = 0;

    char* val_str = strtok(NULL, " ");
    if(!val_str)
    {
        debug_error("Command `%s` expects value argument.");
        errno = EIO;
        return 0;
    }

    if(val_str[0] != '0' || val_str[1] != 'x')
    {
        debug_error("Value argument `%s` does not match `0x[0-9a-fA-F]+`", val_str);
        errno = EINVAL;
        return 0;
    }

    return strtoul(val_str, NULL, 16);
}

static bool is_enabled(int* enabled, bool enabled_when_true)
{
    return enabled_when_true 
        ? *enabled ? true : false 
        : *enabled ? false : true;
}

static void handle_help(const char* input)
{
    printf(help_text_header, global.debugger.bin_file, global.debugger.src_file);
    for(size_t i = 0; cmds[i].cmd; i++)
        printf(help_cmd_fmt, 
            is_enabled(cmds[i].enabled, cmds[i].enabled_when_true) ? COLOR_GREEN : COLOR_RED,
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

static bool load_exec(char* exec)
{
    List_T* exec_args = init_list();
    list_push(exec_args, exec);
    if(!exec) {
        debug_error("Command `load` requires at least one argument, got 0.");
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
        
        for(size_t i = 0; i < global.debugger.breakpoints->size; i++)
            ((Breakpoint_T*) global.debugger.breakpoints->items[i])->enabled = false;
    }
    else 
    {
        debug_error("Failed to fork process, got pid `%d` in return.", pid);
        goto fail;
    }

    return true;

fail:
    free_list(exec_args);
    return false;
}

static void handle_sh(const char* input)
{
    input += (uintptr_t) 3;
    system(input);
}

static void handle_load(const char* input)
{
    if(global.debugger.loaded)
    {
        debug_error("An executable is already loaded, please unload first. (pid %d)", global.debugger.loaded);
        return;
    }

   // if(global.debugger.loaded_cmd)
   //     free(global.debugger.loaded_cmd);

    global.debugger.loaded_cmd = strdup(input);
    char* args = strdup(input);

    strtok(args, " ");

    char* exec = strtok(NULL, " ");

    if(exec && strcmp(exec, "help") == 0)
        fprintf(OUTPUT_STREAM, load_help_text, global.debugger.loaded ? COLOR_RED : COLOR_GREEN);
    else
        load_exec(exec);
    
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

static inline u64 debugger_get_pc(void) 
{
    return get_register_value(global.debugger.loaded, REG_RIP);
}

static inline void debugger_set_pc(u64 pc)
{
    set_register_value(global.debugger.loaded, REG_RIP, pc);
}

static void wait_for_signal(void)
{
    i32 wait_status, options = 0;
    waitpid(global.debugger.loaded, &wait_status, options);
}

static void step_over_breakpoint(void)
{
    u64 possible_breakpoint_location = debugger_get_pc() - 1;

    if(global.debugger.breakpoints->size)
    {
        Breakpoint_T* bp = find_breakpoint(possible_breakpoint_location);

        if(bp && bp->enabled)
        {
            u64 prev_instruction_addr = possible_breakpoint_location;
            debugger_set_pc(prev_instruction_addr);

            bool silent = global.silent;
            global.silent = true;
            
            breakpoint_disable(bp);
            ptrace(PTRACE_SINGLESTEP, global.debugger.loaded, NULL, NULL);
            wait_for_signal();
            breakpoint_enable(bp);

            global.silent = silent;
        }
    }
}

static void handle_continue(const char* input)
{
    if(!global.debugger.loaded)
    {
       debug_error("`cont` is only available if an executable is loaded.");
        return; 
    }

    step_over_breakpoint();
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
                debug_info("Process %d hit breakpoint 0x%016lx.", global.debugger.loaded, debugger_get_pc() - 1);
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
        u64 addr = hex_value_arg("bkr add");
        if(errno)
            goto end;

        set_breakpoint_at_address(addr);
    }
    else
    {
        debug_error("Unknown `brk` subcommand `%s`, expect one of [help, list, rm, add].\n        Use `register help` to get help on this command.");
        goto end;
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

        u64 val = hex_value_arg("register write <reg>");
        if(errno)
            goto fail;

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

static inline u64 debugger_read_memory(u64 address)
{
    return ptrace(PTRACE_PEEKDATA, global.debugger.loaded, address, NULL);
}

static inline void debugger_write_memory(u64 address, u64 value)
{
    ptrace(PTRACE_POKEDATA, global.debugger.loaded, address, value);
}

static void handle_memory(const char* input)
{
    char* args = strdup(input);
    strtok(args, " "); // skip `register`

    char* subcommand = strtok(NULL, " ");
    if(!subcommand)
    {
        debug_error("`memory` expects one argument of [help, read, write]");
        goto fail;
    }

    if(strcmp(subcommand, "help") == 0)
        fprintf(OUTPUT_STREAM, memory_help_text, global.debugger.loaded ? COLOR_GREEN : COLOR_RED);
    else if(!global.debugger.loaded) 
    {
        debug_error("`memory` is only available if an executable is loaded.");
        goto fail;
    }
    else if(strcmp(subcommand, "read") == 0)
    {
        u64 addr = hex_value_arg("memory read");
        if(errno)
            goto fail;
        
        fprintf(OUTPUT_STREAM, "0x%016lx: 0x%016lx\n", addr, debugger_read_memory(addr));
    }
    else if(strcmp(subcommand, "write") == 0)
    {
        u64 addr = hex_value_arg("memory write");
        if(errno)
            goto fail;
        
        u64 value = hex_value_arg("memory write <addr>");
        if(errno)
            goto fail;
        
        fprintf(OUTPUT_STREAM, "0x%016lx: 0x%016lx -> 0x%016lx\n", addr, debugger_read_memory(addr), value);
        debugger_write_memory(addr, value);
    }
    else 
    {
        debug_error("Unknown `memory` subcommand `%s`, expect one of [help, read, write].\n        Use `memory help` to get help on this command.");
        goto fail;
    }

fail:
    free(args);
}