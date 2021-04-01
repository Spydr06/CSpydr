#ifndef CSPYDR_ACTION_TREE_H
#define CSPYDR_ACTION_TREE_H

#include "../list.h"
#include "AST.h"

typedef enum ACT_BASIC_DATA_TYPE
{
    ACT_I8,
    ACT_I16,
    ACT_I32,
    ACT_I64,

    ACT_U8,
    ACT_U16,
    ACT_U32,
    ACT_U64,

    ACT_F32,
    ACT_F64,

    ACT_VOID,
    ACT_BOOL,
    ACT_CHAR,
    ACT_STR,

    ACT_VEC,

    ACT_NIL,
    ACT_UNDEF,
} ACTBasicDataType_T;

typedef struct ACT_ROOT_STRUCT
{
    //TODO: list_T* customDataTypes;
    list_T* globals;
    list_T* functions;
} ACTRoot_T;

typedef struct ACT_DATA_TYPE ACTDataType_T;

struct ACT_DATA_TYPE
{
    ACTBasicDataType_T basicType;
    ACTDataType_T* innerType;
};

typedef struct ACT_GLOBAL_STRUCT
{
    char* name;
    ACTDataType_T* dataType;
} ACTGlobal_T;

typedef struct ACT_LOCAL_STRUCT
{
    char* name;
    ACTDataType_T* dataType;
} ACTLocal_T;

typedef struct ACT_COMPOUND_STRUCT
{
    list_T* locals;
    list_T* body;
} ACTCompound_T;

typedef struct ACT_FUNCTION_STRUCT
{
    char* name;
    ACTDataType_T* returnType;
    list_T* args;

    ACTCompound_T* body;
} ACTFunction_T;

ACTRoot_T* initACTRoot();

ACTFunction_T* initACTFunction(char* name, AST_T* returnType);
ACTDataType_T* initACTDataType(AST_T* ast);
ACTGlobal_T* initACTGlobal(char* name, AST_T* dataType);

void registerFunction(ACTRoot_T* root, AST_T* func);
void registerGlobal(ACTRoot_T* root, AST_T* global);

#endif