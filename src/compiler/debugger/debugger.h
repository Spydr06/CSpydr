#ifndef CSPYDR_DBG_H
#define CSPYDR_DBG_H

#include <stdbool.h>
#include <stdio.h>

#include "config.h"
#include "list.h"

typedef struct DEBUGGER_STRUCT Debugger_T;
typedef struct COMMAND_STRUCT {
    char* cmd;
    void (*fn)(Debugger_T*, const char*);
    int* enabled;
    bool enabled_when_true;
    const char* description;
} Command_T;

struct DEBUGGER_STRUCT {
    Context_T* context;

    char* bin_file;
    char* src_file;
    
    List_T* breakpoints;
    
    char* loaded_cmd;
    pid_t loaded;
    
    bool running;
    char prompt[BUFSIZ];

    Command_T* commands;
};

void debug_info(const char* fmt, ...);
void debug_error(const char* fmt, ...);
void debug_repl(Context_T* context, const char* src, const char* bin);

#endif