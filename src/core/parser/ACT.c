#include "ACT.h"
#include "../../log.h"

#include <string.h>

ACTRoot_T* initACTRoot()
{
    ACTRoot_T* act = calloc(1, sizeof(struct ACT_ROOT_STRUCT));

    act->functions = initList(sizeof(struct ACT_FUNCTION_STRUCT*));
    act->globals = initList(sizeof(struct ACT_GLOBAL_STRUCT*));

    return act;
}

ACTDataType_T* initACTDataType(AST_T* ast)
{
    ACTDataType_T* act = calloc(1, sizeof(struct ACT_DATA_TYPE));

    ACTBasicDataType_T basicDataType;
    ACTDataType_T* subtype = NULL;
    switch(ast->dataType->type)
    {
        case I8:
            basicDataType = ACT_I8;
            break;
        case I16:
            basicDataType = ACT_I16;
            break;
        case I32:
            basicDataType = ACT_I32;
            break;
        case I64:
            basicDataType = ACT_I64;
            break;

        case U8:
            basicDataType = ACT_U8;
            break;
        case U16:
            basicDataType = ACT_U16;
            break;
        case U32:
            basicDataType = ACT_U32;
            break;
        case U64:
            basicDataType = ACT_U64;
            break;

        case F32:
            basicDataType = ACT_F32;
            break;
        case F64:
            basicDataType = ACT_F64;
            break;
        
        case BOOL:
            basicDataType = ACT_BOOL;
            break;
        case CHAR:
            basicDataType = ACT_CHAR;
            break;
        case STR:
            basicDataType = ACT_STR;
            break;

        case VEC:
            basicDataType = ACT_VEC;
            subtype = initACTDataType(ast->dataType->subtype);
            break;

        case VOID:
            basicDataType = ACT_VOID;
            break;

        default:
            basicDataType = ACT_UNDEF;
            break;
    }

    act->basicType = basicDataType;
    act->innerType = subtype;

    return act;
}

ACTFunction_T* initACTFunction(char* name, AST_T* returnType)
{
    ACTFunction_T* act = calloc(1, sizeof(struct ACT_FUNCTION_STRUCT));

    act->name = name;
    act->args = initList(sizeof(struct ACT_LOCAL_STRUCT*));
    act->returnType = initACTDataType(returnType);

    return act;
}

ACTGlobal_T* initACTGlobal(char* name, AST_T* dataType)
{
    ACTGlobal_T* act = calloc(1, sizeof(struct ACT_GLOBAL_STRUCT));
    act->name = name;
    act->dataType = initACTDataType(dataType);

    return act;
}

void registerFunction(ACTRoot_T* root, AST_T* func)
{   
    char* funcName = func->def->name;

    for(int i = 0; i < root->functions->size; i++)
    {
        ACTFunction_T* currentFunc = (ACTFunction_T*) root->functions->items[i];

        if(strcmp(funcName, currentFunc->name) == 0)
        {
            LOG_ERROR("Multiple definitions of function \"%s\"!n", funcName);
            exit(1);
        }
    }

    listPush(root->functions, initACTFunction(funcName, func->def->dataType));
}

void registerGlobal(ACTRoot_T* root, AST_T* global)
{
    char* globalName = global->def->name;

    for(int i = 0; i < root->globals->size; i++)
    {
        ACTGlobal_T* currentGlobal = (ACTGlobal_T*) root->globals->items[i];

        if(strcmp(globalName, currentGlobal->name) == 0)
        {
            LOG_ERROR("Multiple definitions of global variable \"%s\"\n", globalName);
            exit(1);
        }
    }

    listPush(root->globals, initACTGlobal(globalName, global->def->dataType));
}