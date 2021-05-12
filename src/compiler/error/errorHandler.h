#ifndef CSPYDR_ERROR_HANDLER_H
#define CSPYDR_ERROR_HANDLER_H

#include "../list.h"
#include <stdbool.h>

#include "../io/file.h"

typedef enum ERROR_TYPE
{   
    ERR_SYNTAX_ERROR,
    ERR_SYNTAX_WARNING,
    ERR_REDEFINITION,
    ERR_UNDEFINED,

    ERR_ILLEGAL_TYPE_CAST,
    ERR_TYPE_CAST_WARN,

    ERR_INTERNAL,
} errorType_T;

typedef struct ERROR_MESSAGE
{
    errorType_T type;

    bool forceExit;
    char* message;
} errorMessage_T;

typedef struct ERROR_HANDLER
{
    srcFile_T* file;
    list_T* errorMessages;

    bool exitAfterParsing;
} errorHandler_T;

errorHandler_T* initErrorHandler();
void freeErrorHandler(errorHandler_T* eh);
errorMessage_T* initErrorMessage(errorType_T type, unsigned int lineNumber, bool forceExit, char* message);
void freeErrorMessage(errorMessage_T* em);

void throwError(errorHandler_T* hander, errorMessage_T* message);
void throwSyntaxError(errorHandler_T* handler, const char* message, unsigned int lineNumber, unsigned int character);
void throwRedefinitionError(errorHandler_T* handler, const char* message, unsigned int lineNumber, unsigned int character);
void throwUndefinitionError(errorHandler_T* handler, const char* message, unsigned int lineNumber, unsigned int character);

#endif