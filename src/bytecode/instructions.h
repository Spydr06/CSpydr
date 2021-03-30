#ifndef CSPYDR_BYTECODE_INSTRUCTIONS_H
#define CSPYDR_BYTECODE_INSTRUCTIONS_H

#include "../core/list.h"

typedef enum BYTECODE_INSTRUCTION_TYPE_ENUM
{
    OP_FN,
    OP_ARG,

    OP_LOCAL,
    OP_GLOBAL,

    OP_CALL,
    OP_RET,
    OP_EXIT,

    OP_IF,
    OP_ELSE,
    OP_IF_END,

    OP_CONST,
    OP_CHAR,
    OP_STR,
    OP_BOOL,

    OP_SET,
    OP_ADD,
    OP_SUB,
    OP_MULT,
    OP_DIV,
} BCInstructionType_T;

typedef struct BYTECODE_INSTRUCTION_STRUCT
{
    BCInstructionType_T type;
    list_T* args;
} BCInstruction_T;

BCInstruction_T* initInstruction0(BCInstructionType_T type);
BCInstruction_T* initInstruction1(BCInstructionType_T type, char* a);
BCInstruction_T* initInstruction2(BCInstructionType_T type, char* a, char* b);
char* BCInstructionToString(BCInstruction_T* instruction);

#endif
