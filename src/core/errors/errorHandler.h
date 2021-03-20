#ifndef CSPYDR_ERROR_HANDLER_H
#define CSPYDR_ERROR_HANDLER_H

#include "../list.h"
#include <stdbool.h>

typedef enum ERROR_TYPE
{   
    ERR_SYNTAX_ERROR,
    ERR_SYNTAX_WARNING,

    ERR_ILLEGAL_TYPE_CAST,
    ERR_TYPE_CAST_WARN,

    ERR_UNDEFINED,
    ERR_INTERNAL,
} errorType_T;

// a wrapper around a C string, because my list implementation has problems with bare srings
typedef struct ERROR_SRC_LINE
{
    char* line;
} srcLine_T;

typedef struct ERROR_MESSAGE
{
    errorType_T type;
    unsigned int lineNumber;

    bool forceExit;
    char* message;
} errorMessage_T;

typedef struct ERROR_HANDLER
{
    list_T* linesInSrcFile;
    list_T* errorMessages;
} errorHandler_T;

errorHandler_T* initErrorHandler();
srcLine_T* initSrcLine(char* str);
errorMessage_T* initErrorMessage(errorType_T type, unsigned int lineNumber, bool forceExit, char* message);

void pushSrcLine(errorHandler_T* handler, char* line);

void throwError(errorHandler_T* hander, errorMessage_T* message);
void throwSyntaxError(errorHandler_T* handler, const char* message, const char* srcPath, unsigned int lineNumber, unsigned int character);

#endif