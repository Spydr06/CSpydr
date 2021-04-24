#include "errorHandler.h"
#include "../io/log.h"

#include <string.h>
#include <stdio.h>

#define LINE_NUMBER_SPACES 4

errorHandler_T* initErrorHandler(srcFile_T* file)
{
    errorHandler_T* handler = calloc(1, sizeof(struct ERROR_HANDLER));
    handler->errorMessages = initList(sizeof(struct ERROR_MESSAGE*));
    handler->exitAfterParsing = false;
    handler->file = file;

    if(handler->file == NULL)
    {
        LOG_ERROR("file is null\n");
        exit(1);
    }

    return handler;
}

void freeErrorHandler(errorHandler_T* eh)
{
    for(int i = 0; i < eh->errorMessages->size; i++)
    {
        freeErrorMessage((errorMessage_T*) eh->errorMessages->items[i]);
    }
    freeList(eh->errorMessages);
    free(eh);
}

errorMessage_T* initErrorMessage(errorType_T type, unsigned int lineNumber, bool forceExit, char* message)
{
    errorMessage_T* error = calloc(1, sizeof(struct ERROR_MESSAGE));
    error->type = type;
    error->forceExit = forceExit;
    error->message = message;

    return error;
}

void freeErrorMessage(errorMessage_T* em) 
{
    free(em->message);
    free(em);
}

void throwError(errorHandler_T* handler, errorMessage_T* message)
{
    switch(message->type)
    {
        case ERR_SYNTAX_ERROR:
        case ERR_ILLEGAL_TYPE_CAST:
            LOG_ERROR_F("%s\n", message->message);
            break;
        case ERR_REDEFINITION:
            handler->exitAfterParsing = true;
            LOG_ERROR_F("%s\n", message->message);
            break;
        case ERR_SYNTAX_WARNING:
        case ERR_TYPE_CAST_WARN:
            LOG_WARN_F("%s\n", message->message);
            break;
        case ERR_UNDEFINED:
            LOG_ERROR_F("%s\n", message->message);
            break;
        case ERR_INTERNAL:  // this should never happen
            LOG_ERROR_F("%s\n", message->message);
            break;
        default:
            LOG_ERROR_F("Undefined error: %d =!=> %s\n", message->type, message->message);
            break;
    }

    if(message->forceExit) {
        //TODO: exit safely
        freeErrorHandler(handler);
        exit(1);
    }

    // if no termination is needed, push the error onto a stack for later debugging
    listPush(handler->errorMessages, message);
}

void throwSyntaxError(errorHandler_T* handler, const char* message, unsigned int lineNumber, unsigned int character)
{
    // original error message macro
    /*LOG_ERROR(COLOR_BOLD_WHITE "%s:%d:%d " COLOR_RESET "=>" COLOR_BOLD_RED " [Error]" COLOR_RESET " %s\n %*d | %s\n %*s | " COLOR_BOLD_RED "%*s%s\n",  \
                                parser->lexer->srcPath, parser->lexer->line, parser->lexer->iInLine, msg, LINE_NUMBER_SPACES, parser->lexer->line,      \
                                parser->lexer->currentLine, LINE_NUMBER_SPACES, "", parser->lexer->iInLine, "^~", "here");*/

    // new error message generator code
    const char* template = COLOR_BOLD_WHITE "%s:%d:%d " COLOR_RESET "=>" COLOR_BOLD_RED " [syntax]" COLOR_RESET 
                           " %s\n %*d | %s %*s | " COLOR_BOLD_BLUE "%*shere\n";
    char* lineCode = getLine(handler->file, lineNumber);
    const char* pointer = "^~";

    //generate the message
    char* value = calloc(strlen(template) + strlen(handler->file->path) + strlen(message) + strlen(pointer) + strlen(lineCode) + 128, sizeof(char));
    sprintf(value, template, handler->file->path, lineNumber + 1, character, message, LINE_NUMBER_SPACES, lineNumber + 1, lineCode, LINE_NUMBER_SPACES, "", character, "^~");

    errorMessage_T* error = initErrorMessage(ERR_SYNTAX_ERROR, lineNumber, true, value); //generate the error struct
    throwError(handler, error); //submit the error
    free(value);
}

void throwRedefinitionError(errorHandler_T* handler, const char* message, unsigned int lineNumber, unsigned int character)
{
    const char* template = COLOR_BOLD_WHITE "%s:%d:%d " COLOR_RESET "=>" COLOR_BOLD_RED " [redef]" COLOR_RESET 
                           " %s\n %*d | %s %*s | " COLOR_BOLD_BLUE "%*shere\n";
    char* lineCode = getLine(handler->file, lineNumber);
    const char* pointer = "^~";

    //generate the message
    char* value = calloc(strlen(template) + strlen(handler->file->path) + strlen(message) + strlen(pointer) + strlen(lineCode) + 128, sizeof(char));
    sprintf(value, template, handler->file->path, lineNumber + 1, character, message, LINE_NUMBER_SPACES, lineNumber + 1, lineCode, LINE_NUMBER_SPACES, "", character, "^~");

    errorMessage_T* error = initErrorMessage(ERR_REDEFINITION, lineNumber, false, value); //generate the error struct
    throwError(handler, error); //submit the error
    free(value);
}

void throwUndefinitionError(errorHandler_T* handler, const char* message, unsigned int lineNumber, unsigned int character)
{
    const char* template = COLOR_BOLD_WHITE "%s:%d:%d " COLOR_RESET "=>" COLOR_BOLD_RED " [undef]" COLOR_RESET 
                           " %s\n %*d | %s %*s | " COLOR_BOLD_BLUE "%*shere\n";
    char* lineCode = getLine(handler->file, lineNumber);
    const char* pointer = "^~";

    //generate the message
    char* value = calloc(strlen(template) + strlen(handler->file->path) + strlen(message) + strlen(pointer) + strlen(lineCode) + 128, sizeof(char));
    sprintf(value, template, handler->file->path, lineNumber + 1, character, message, LINE_NUMBER_SPACES, lineNumber + 1, lineCode, LINE_NUMBER_SPACES, "", character, "^~");

    errorMessage_T* error = initErrorMessage(ERR_UNDEFINED, lineNumber, true, value); //generate the error struct
    throwError(handler, error); //submit the error
    free(value);
}