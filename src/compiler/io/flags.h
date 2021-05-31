#ifndef CSPYDR_FLAGS_H
#define CSPYDR_FLAGS_H

#include "../list.h"

typedef enum FLAG_TYPE
{
    FLAG_HELP,
    FLAG_VERSION,
    FLAG_RUN,
    FLAG_OUTPUT,
    FLAG_DEBUG,
    FLAG_INPUT,
    FLAG_INFO,
    FLAG_UNDEFINED,
    FLAG_ENABLE_TRANSPILING
} FlagType_T;

typedef struct FLAG_STRUCT
{
    FlagType_T type;
    char* value;
} Flag_T;

typedef struct FLAG_DISPATCHER_STRUCT
{
    List_T* flags;
} FlagDispatcher_T;

Flag_T* init_flag(FlagType_T type, char* value);
void    free_flag(Flag_T* flag);
FlagDispatcher_T* dispatch_flags(int argc, char* argv[]);
void              free_flagdispatcher(FlagDispatcher_T* flag);

#endif