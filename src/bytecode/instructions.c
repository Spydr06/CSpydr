#include "instructions.h"
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

BCInstruction_T* initInstruction(BCInstructionType_T type, unsigned int argc, ...)
{
    BCInstruction_T* instruction = calloc(1, sizeof(struct BYTECODE_INSTRUCTION_STRUCT));
    instruction->type = type;
    instruction->args = initList(sizeof(char*)); //FIXME

    va_list argv;
    va_start(argv, argc);

    for(int i = 0; i < argc; i++)
    {
        listPush(instruction->args, va_arg(argv, char*));
    }
    va_end(argv);

    return instruction;
}

static const char* BCInstructionTypeToString(BCInstructionType_T instruction)
{
    switch(instruction)
    {
        case OP_DEF_FN: return "OP_DEF_FN";
        case OP_LOCAL: return "OP_LOCAL";
        case OP_GLOBAL: return "OP_GLOBAL";
        case OP_CALL: return "OP_CALL";
        case OP_RET: return "OP_RET";
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
    const char* template = " BC | %s: %s";
    const char* typeStr = BCInstructionTypeToString(instruction->type);

    char* argStr = calloc(1, sizeof(char));
    for(int i = 0; i < instruction->args->size; i++)
    {
        char* next = instruction->args->items[i];
        printf("%s\n", next);
        char* old = argStr;
        argStr = realloc(argStr, (strlen(argStr) + strlen(next) + 128) * sizeof(char));
        sprintf(argStr, "%s, %s", old, next);
    }
    if(strlen(argStr) > 2) 
    {
        argStr += 2;
    }

    char* value = calloc(strlen(template) + strlen(typeStr) + strlen(argStr) + 1, sizeof(char));
    sprintf(value, template, typeStr, argStr);

    return value;
}