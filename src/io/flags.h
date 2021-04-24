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
} flagType_T;

typedef struct FLAG_STRUCT
{
    flagType_T type;
    char* value;
} flag_T;

typedef struct FLAG_DISPATCHER_STRUCT
{
    list_T* flags;
} flagDispatcher_T;

flag_T* initFlag(flagType_T type, char* value);
void freeFlag(flag_T* flag);
flagDispatcher_T* dispatchFlags(int argc, char* argv[]);
void freeFlagDispatcher(flagDispatcher_T* flag);

#endif