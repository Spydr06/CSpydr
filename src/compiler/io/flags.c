#include "flags.h"
#include "../list.h"
#include <string.h>
#include <stdio.h>
#include "log.h"

Flag_T* init_flag(FlagType_T type, char* value)
{
    Flag_T* flag = calloc(1, sizeof(struct FLAG_STRUCT));
    flag->type = type;

    if(value != NULL)
    {
        flag->value = calloc(strlen(value) + 1, sizeof(char*));
        strcpy(flag->value, value);
    }
    return flag;
}

void freeFlag(Flag_T* flag) {
    free(flag->value);
    free(flag);
}

FlagDispatcher_T* dispatch_flags(int argc, char* argv[])
{
    FlagDispatcher_T* dispatcher = calloc(1, sizeof(struct FLAG_DISPATCHER_STRUCT));
    dispatcher->flags = init_list(sizeof(struct FLAG_STRUCT*));

    if(argc == 0)
    {
        list_push(dispatcher->flags, init_flag(FLAG_HELP, NULL));
    }

    for(int i = 1; i < argc; i++)
    {
        char* arg = argv[i];        

        if(strcmp(arg, "-h") == 0 || strcmp(arg, "--help") == 0)
        {
            list_push(dispatcher->flags, init_flag(FLAG_HELP, NULL));
        }
        else if(strcmp(arg, "-o") == 0 || strcmp(arg, "--out") == 0 || strcmp(arg, "--output") == 0)
        {
            list_push(dispatcher->flags, init_flag(FLAG_OUTPUT, argv[i++ + 1]));
        }
        else if(strcmp(arg, "-v") == 0 || strcmp(arg, "--version") == 0)
        {
            list_push(dispatcher->flags, init_flag(FLAG_VERSION, NULL));
        }
        else if(strcmp(arg, "-d") == 0 || strcmp(arg, "--debug") == 0)
        {
            list_push(dispatcher->flags, init_flag(FLAG_DEBUG, NULL));
        }
        else if(strcmp(arg, "-i") == 0 || strcmp(arg, "--info") == 0)
        {
            list_push(dispatcher->flags, init_flag(FLAG_INFO, NULL));
        }
        else if(strcmp(arg, "-t") == 0 || strcmp(arg, "--transpile") == 0)
        {
            list_push(dispatcher->flags, init_flag(FLAG_ENABLE_TRANSPILING, NULL));
        }
        else if(arg[0] == '-')
        {
            LOG_ERROR_F("Unknown parameter \"%s\". Use --help or -h for help.\n", arg);
            exit(1);
        }
        else {
            list_push(dispatcher->flags, init_flag(FLAG_INPUT, arg));
        }
    }

    return dispatcher;
}

void free_flagdispatcher(FlagDispatcher_T* dispatcher) {
    for(int i = 0; i < dispatcher->flags->size; i++) {
        freeFlag(dispatcher->flags->items[i]);
    }

    free_list(dispatcher->flags);
    free(dispatcher);
}