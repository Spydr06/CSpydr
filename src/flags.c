#include "flags.h"
#include "core/list.h"
#include <string.h>
#include <stdio.h>

flag_T* initFlag(flagType_T type, char* value)
{
    flag_T* flag = calloc(1, sizeof(struct FLAG_STRUCT));
    flag->type = type;

    if(value != NULL)
    {
        flag->value = calloc(strlen(value) + 1, sizeof(char*));
        strcpy(flag->value, value);
    }
    return flag;
}

flagDispatcher_T* dispatchFlags(int argc, char* argv[])
{
    flagDispatcher_T* dispatcher = calloc(1, sizeof(struct FLAG_DISPATCHER_STRUCT));
    dispatcher->flags = initList(sizeof(struct FLAG_STRUCT*));

    if(argc == 0)
    {
        listPush(dispatcher->flags, initFlag(FLAG_HELP, NULL));
    }

    for(int i = 1; i < argc; i++)
    {
        char* arg = argv[i];        

        if(strcmp(arg, "-h") == 0 || strcmp(arg, "-help") == 0)
        {
            listPush(dispatcher->flags, initFlag(FLAG_HELP, NULL));
        }
        else if(strcmp(arg, "-o") == 0 || strcmp(arg, "-out") == 0 || strcmp(arg, "-output") == 0)
        {
            listPush(dispatcher->flags, initFlag(FLAG_OUTPUT, argv[i++]));
        }
        else if(strcmp(arg, "-v") == 0 || strcmp(arg, "-version") == 0)
        {
            listPush(dispatcher->flags, initFlag(FLAG_VERSION, NULL));
        }
        else if(strcmp(arg, "-d") == 0 || strcmp(arg, "-debug") == 0)
        {
            listPush(dispatcher->flags, initFlag(FLAG_DEBUG, NULL));
        }
        else {
            listPush(dispatcher->flags, initFlag(FLAG_INPUT, arg));
        }
    }

    return dispatcher;
}