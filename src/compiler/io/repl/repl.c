#include "repl.h"

#include "../../globals.h"
#include "../log.h"
#include "../file.h"
#include "../io.h"
#include "../../toolchain.h"
#include "../../version.h"

#include "../../platform/platform_bindings.h"

#include <stdlib.h>
#include <string.h>

#define INPUT_STREAM stdin

#ifndef __GLIBC__
char *strsep(char **stringp, const char *delim) 
{
    if (*stringp == NULL) { return NULL; }
    char *token_start = *stringp;
    *stringp = strpbrk(token_start, delim);
    if (*stringp) {
        **stringp = '\0';
        (*stringp)++;
    }
    return token_start;
}
#endif

static void print_prompt();
static void handle_input(char* input);

static void handle_quit(char* input);
static void handle_run(char* input);
static void handle_edit(char* input);
static void handle_help(char* input);
static void handle_clear(char* input);

static struct 
{
    char* cmd;
    void (*fn)(char* input);
} commands[] = 
{
    {"quit", handle_quit},
    {"q",    handle_quit},
    {"run",  handle_run},
    {"r",    handle_run},
    {"edit", handle_edit},
    {"e",    handle_edit},
    {"help", handle_help},
    {"h",    handle_help},
    {"cls",  handle_clear},
    {NULL, NULL}
};

bool repl_running = false;

#define DEFAULT_EDITOR "nano"

static const char* greeting = COLOR_BOLD_MAGENTA "CSpydr REPL %s\n" COLOR_RESET
                       "Copyright (c) 2021 Spydr06\n"
                       "CSpydr is licensed under the MIT License.\n"
                       "For help, type `:help`\n";

static const char* help_text = 
                       "Type one of the following Commands or valid CSpydr code for execution:\n"
                       COLOR_BOLD_WHITE "  quit, q" COLOR_RESET ": Quit the CSpydr REPL.\n"
                       COLOR_BOLD_WHITE "  run,  r" COLOR_RESET ": Run the previously edited code.\n"
                       COLOR_BOLD_WHITE "  edit, e" COLOR_RESET ": Edit the CSpydr Code in a proper text editor.\n"
                       "           (This uses the $EDITOR environment variable,\n"
                       "           if it is not set, CSpydr will default to `" DEFAULT_EDITOR "`.)\n"
                       COLOR_BOLD_WHITE "  cls,  c" COLOR_RESET ": clear the console output.\n"
                       COLOR_BOLD_WHITE "  help, h" COLOR_RESET ": Show this help text.\n";

void repl()
{
    // print the REPL greeting
    fprintf(OUTPUT_STREAM, greeting, get_cspydr_version());

    silent = true;  // disable compiler output
    repl_running = true;
 
    // enter the main loop
    while(repl_running)
    {
        print_prompt();

        char input[__CSP_MAX_REPL_CMD_LEN] = { '\0' };
        if(!fgets(input, __CSP_MAX_REPL_CMD_LEN, INPUT_STREAM)) {
            LOG_ERROR("\nError getting REPL input from stdin.\n");
            continue;
        }

        size_t input_len;
    newline:
        input_len = strlen(input);
        if(input[input_len - 2] == '\\')
        {
            input[input_len - 2] = '\n';
            if(!fgets(input + input_len - 1, __CSP_MAX_REPL_CMD_LEN, INPUT_STREAM)) {
                LOG_ERROR("\nError getting REPL input from stdin.\n");
                continue;
            }
            goto newline;
        }

        handle_input(input);
    }    
}

static void print_prompt()
{
    fprintf(OUTPUT_STREAM, "%s[%d]" COLOR_RESET " >> ", last_exit_code == 0 ? COLOR_BOLD_WHITE : COLOR_BOLD_RED, last_exit_code);
    fflush(OUTPUT_STREAM);
}

static void handle_input(char* input)
{
    if(strcmp(input, "\n") == 0)
        return;
    else if(input[0] == ':')
    {
        input[strlen(input) - 1] = '\0';

        for(int i = 0; commands[i].cmd != NULL; i++)
            if(strcmp(input + 1, commands[i].cmd) == 0)
            {
                commands[i].fn(input);
                return;
            }
        LOG_ERROR_F("Unknown REPL argument `%s`.\n", input);
    }
    else {
        char path[BUFSIZ] = { '\0' };
        sprintf(path, "%s" DIRECTORY_DELIMS CACHE_DIR DIRECTORY_DELIMS "repl.csp", get_home_directory());
    
        write_file(path, input);
    
        compile(path, DEFAULT_OUTPUT_FILE, AC_RUN);
    }
}

static void handle_quit(char* input)
{
    repl_running = false;
}

static void handle_run(char* input)
{
    char path[BUFSIZ] = { '\0' };
    sprintf(path, "%s" DIRECTORY_DELIMS CACHE_DIR DIRECTORY_DELIMS "repl.csp", get_home_directory());
    compile(path, DEFAULT_OUTPUT_FILE, AC_RUN);
}

static void handle_edit(char* input)
{
    char path[BUFSIZ] = { '\0' };
    sprintf(path, "%s" DIRECTORY_DELIMS CACHE_DIR DIRECTORY_DELIMS "repl.csp", get_home_directory());

#ifdef __linux
    char* editor = getenv("EDITOR");

    if(strcmp(editor, "") == 0)
        editor = DEFAULT_EDITOR;
#elif defined(_WIN32)
    char* editor = "notepad"
#endif

    char* const args[] = 
    {
        editor,
        path,
        NULL
    };

    int exit_code;
    if((exit_code = subprocess(editor, args, false)) == -1)
        LOG_ERROR_F("Error running `%s` as subprocess (exit code %d)", editor, exit_code);
}

static void handle_help(char* input)
{
    fprintf(OUTPUT_STREAM, "%s", help_text);
}

static void handle_clear(char* input)
{
    clrscr();
}