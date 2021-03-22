#include "errorHandler.h"
#include "../../log.h"

#include <string.h>
#include <stdio.h>

#define LINE_NUMBER_SPACES 4

errorHandler_T* initErrorHandler()
{
    errorHandler_T* handler = calloc(1, sizeof(struct ERROR_HANDLER));
    handler->currentLine = calloc(1, sizeof(char));
    handler->errorMessages = initList(sizeof(struct ERROR_MESSAGE*));

    return handler;
}

srcLine_T* initSrcLine(char* str)
{
    srcLine_T* line = calloc(1, sizeof(struct ERROR_SRC_LINE));
    line->line = calloc(strlen(str) + 1, sizeof(char));
    strcpy(line->line, str);
    return line;
}

errorMessage_T* initErrorMessage(errorType_T type, unsigned int lineNumber, bool forceExit, char* message)
{
    errorMessage_T* error = calloc(1, sizeof(struct ERROR_MESSAGE));
    error->type = type;
    error->lineNumber = lineNumber;
    error->forceExit = forceExit;
    error->message = message;

    return error;
}

void pushSrcLine(errorHandler_T* handler, char* line)
{
    handler->currentLine = calloc(strlen(line) + 1, sizeof(char));
    strcpy(handler->currentLine, line);
}

void throwError(errorHandler_T* handler, errorMessage_T* message)
{
    switch(message->type)
    {
        case ERR_SYNTAX_ERROR:
        case ERR_ILLEGAL_TYPE_CAST:
            LOG_ERROR("%s\n", message->message);
            break;
        case ERR_SYNTAX_WARNING:
        case ERR_TYPE_CAST_WARN:
            LOG_WARN("%s\n", message->message);
            break;
        case ERR_UNDEFINED:
            LOG_INFO("%s\n", message->message);
            break;
        case ERR_INTERNAL:  // this should never happen
            LOG_ERROR("%s\n", message->message);
            break;
        default:
            LOG_ERROR("Undefined error: %d =!=> %s\n", message->type, message->message);
            break;
    }

    if(message->forceExit) {
        //TODO: exit safely
        exit(1);
    }

    // if no termination is needed, push the error onto a stack for later debugging
    listPush(handler->errorMessages, message);
}

void throwSyntaxError(errorHandler_T* handler, const char* message, const char* srcPath, unsigned int lineNumber, unsigned int character)
{
    // original error message macro
    /*LOG_ERROR(COLOR_BOLD_WHITE "%s:%d:%d " COLOR_RESET "=>" COLOR_BOLD_RED " [Error]" COLOR_RESET " %s\n %*d | %s\n %*s | " COLOR_BOLD_RED "%*s%s\n",  \
                                parser->lexer->srcPath, parser->lexer->line, parser->lexer->iInLine, msg, LINE_NUMBER_SPACES, parser->lexer->line,      \
                                parser->lexer->currentLine, LINE_NUMBER_SPACES, "", parser->lexer->iInLine, "^~", "here");*/

    // new error message generator code
    const char* template = COLOR_BOLD_WHITE "%s:%d:%d " COLOR_RESET "=>" COLOR_BOLD_RED " [Error]" COLOR_RESET 
                           " %s\n %*d | %s\n %*s | " COLOR_BOLD_BLUE "%*shere\n";
    char* lineCode = handler->currentLine;
    const char* pointer = "^~";

    //generate the message
    char* value = calloc(strlen(template) + strlen(srcPath) + strlen(message) + strlen(pointer) + strlen(lineCode) + 128, sizeof(char));
    sprintf(value, template, srcPath, lineNumber, character, message, LINE_NUMBER_SPACES, lineNumber, lineCode, LINE_NUMBER_SPACES, "", character, "^~");

    errorMessage_T* error = initErrorMessage(ERR_SYNTAX_ERROR, lineNumber, true, value); //generate the error struct
    throwError(handler, error); //submit the error
}