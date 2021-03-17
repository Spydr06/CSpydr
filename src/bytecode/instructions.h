#ifndef CSPYDR_BYTECODE_INSTRUCTIONS_H
#define CSPYDR_BYTECODE_INSTRUCTIONS_H

#include "../core/list.h"

typedef enum BYTECODE_INSTRUCTION_TYPE_ENUM
{
    OP_DEF_FN,

    OP_LOCAL,
    OP_GLOBAL,

    OP_CALL,
    OP_RET,

    OP_JMP_IF,
    OP_JMP,

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

BCInstruction_T* initInstruction(BCInstructionType_T type, unsigned int argc, ...);
char* BCInstructionToString(BCInstruction_T* instruction);

#endif