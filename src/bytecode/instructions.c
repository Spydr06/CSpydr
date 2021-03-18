#include "instructions.h"
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "../core/list.h"

BCInstruction_T* initInstruction1(BCInstructionType_T type, char* a)
{
    BCInstruction_T* instruction = calloc(1, sizeof(struct BYTECODE_INSTRUCTION_STRUCT));
    instruction->type = type;
    instruction->args = initList(sizeof(char*));

    listPush(instruction->args, a);

    return instruction;
}

BCInstruction_T* initInstruction2(BCInstructionType_T type, char* a, char* b)
{
    BCInstruction_T* instruction = calloc(1, sizeof(struct BYTECODE_INSTRUCTION_STRUCT));
    instruction->type = type;
    instruction->args = initList(sizeof(char));

    listPush(instruction->args, a);
    listPush(instruction->args, b);

    return instruction;
}

static const char* BCInstructionTypeToString(BCInstructionType_T instruction)
{
    switch(instruction)
    {
        case OP_FN: return "OP_FN";
        case OP_ARG: return "OP_ARG";
        case OP_LOCAL: return "OP_LOCAL";
        case OP_GLOBAL: return "OP_GLOBAL";
        case OP_CALL: return "OP_CALL";
        case OP_RET: return "OP_RET";
        case OP_EXIT: return "OP_EXIT";
        case OP_JMP_IF: return "OP_JMP_IF";
        case OP_JMP: return "OP_JMP";
        case OP_CONST: return "OP_CONST";
        case OP_CHAR: return "OP_CHAR";
        case OP_STR: return "OP_STR";
        case OP_BOOL: return "OP_BOOL";
        case OP_SET: return "OP_SET";
        case OP_ADD: return "OP_ADD";
        case OP_SUB: return "OP_SUB";
        case OP_MULT: return "OP_MULT";
        case OP_DIV: return "OP_DIV";
    }

    return "not stringable";
}

char* BCInstructionToString(BCInstruction_T* instruction)
{
    const char* template = " BC | %s\t %s";
    const char* typeStr = BCInstructionTypeToString(instruction->type);

    char* argStr = calloc(1, sizeof(char));
    for(int i = 0; i < instruction->args->size; i++)
    {
        char* next = instruction->args->items[i];
        char* old = argStr;
        const char* template = "%s,\t%s";
        argStr = realloc(argStr, (strlen(argStr) + strlen(next) + strlen(template) + 1) * sizeof(char));
        sprintf(argStr, template, old, next);
    }
    if(strlen(argStr) > 2) 
    {
        argStr += 2;
    }

    char* value = calloc(strlen(template) + strlen(typeStr) + strlen(argStr) + 1, sizeof(char));
    sprintf(value, template, typeStr, argStr);

    return value;
}