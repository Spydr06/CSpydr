#include "repl.h"

#include "../../globals.h"
#include "../log.h"
#include "../file.h"
#include "../io.h"
#include "../../toolchain.h"

#include "../../platform/platform_bindings.h"

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

static struct 
{
    char* cmd;
    void (*fn)(char* input);
} commands[] = 
{
    {"quit", handle_quit},
    {"q", handle_quit},
    {NULL, NULL}
};

bool repl_running = false;

void repl()
{
    silent = true;  // disable compiler output

    repl_running = true;
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