#include "errorHandler.h"
#include "../io/log.h"

#include <string.h>
#include <stdio.h>

#define LINE_NUMBER_SPACES 4

ErrorHandler_T* init_errorhandler(SrcFile_T* file)
{
    ErrorHandler_T* handler = calloc(1, sizeof(struct ERROR_HANDLER));
    handler->error_messages = init_list(sizeof(struct ERROR_MESSAGE*));
    handler->exit_after_parsing = false;
    handler->file = file;

    if(handler->file == NULL)
    {
        LOG_ERROR("file is null\n");
        exit(1);
    }

    return handler;
}

void free_errorhandler(ErrorHandler_T* eh)
{
    for(int i = 0; i < eh->error_messages->size; i++)
    {
        free_errormessage((ErrorMessage_T*) eh->error_messages->items[i]);
    }
    free_list(eh->error_messages);
    free(eh);
}

ErrorMessage_T* init_errormessage(ErrorType_T type, unsigned int lineNumber, bool force_exit, char* message)
{
    ErrorMessage_T* error = calloc(1, sizeof(struct ERROR_MESSAGE));
    error->type = type;
    error->force_exit = force_exit;
    error->message = message;

    return error;
}

void free_errormessage(ErrorMessage_T* em) 
{
    free(em->message);
    free(em);
}

void throw_error(ErrorHandler_T* handler, ErrorMessage_T* message)
{
    switch(message->type)
    {
        case ERR_SYNTAX_ERROR:
        case ERR_ILLEGAL_TYPE_CAST:
            LOG_ERROR_F("%s", message->message);
            break;
        case ERR_REDEFINITION:
            handler->exit_after_parsing = true;
            LOG_ERROR_F("%s", message->message);
            break;
        case ERR_SYNTAX_WARNING:
        case ERR_TYPE_CAST_WARN:
            LOG_WARN_F("%s", message->message);
            break;
        case ERR_UNDEFINED:
            LOG_ERROR_F("%s", message->message);
            break;
        case ERR_INTERNAL:  // this should never happen
            LOG_ERROR_F("%s", message->message);
            break;
        default:
            LOG_ERROR_F("Undefined error: %d =!=> %s\n", message->type, message->message);
            break;
    }

    if(message->force_exit) {
        //TODO: exit safely
        free_errorhandler(handler);
        exit(1);
    }

    // if no termination is needed, push the error onto a stack for later debugging
    list_push(handler->error_messages, message);
}

void throw_syntax_error(ErrorHandler_T* handler, const char* message, unsigned int line_number, unsigned int character)
{
    const char* template = COLOR_BOLD_WHITE "%s:%d:%d " COLOR_RESET "=>" COLOR_BOLD_RED " [syntax]" COLOR_RESET 
                           " %s\n %*d | %s %*s | " COLOR_BOLD_BLUE "%*shere\n" COLOR_RESET;
    char* line_code = get_line(handler->file, line_number);
    const char* pointer = "^~";

    //generate the message
    char* value = calloc(strlen(template) + strlen(handler->file->path) + strlen(message) + strlen(pointer) + strlen(line_code) + 128, sizeof(char));
    sprintf(value, template, handler->file->path, line_number + 1, character + 1, message, LINE_NUMBER_SPACES, line_number + 1, line_code, LINE_NUMBER_SPACES, "", character + 1, " ^~");

    ErrorMessage_T* error = init_errormessage(ERR_SYNTAX_ERROR, line_number, true, value); //generate the error struct
    throw_error(handler, error); //submit the error
}

void throw_redef_error(ErrorHandler_T* handler, const char* message, unsigned int line_number, unsigned int character)
{
    const char* template = COLOR_BOLD_WHITE "%s:%d:%d " COLOR_RESET "=>" COLOR_BOLD_RED " [redef]" COLOR_RESET 
                           " %s\n %*d | %s %*s | " COLOR_BOLD_BLUE "%*shere\n" COLOR_RESET;
    char* line_code = get_line(handler->file, line_number);
    const char* pointer = "^~";

    //generate the message
    char* value = calloc(strlen(template) + strlen(handler->file->path) + strlen(message) + strlen(pointer) + strlen(line_code) + 128, sizeof(char));
    sprintf(value, template, handler->file->path, line_number + 1, character + 1, message, LINE_NUMBER_SPACES, line_number + 1, line_code, LINE_NUMBER_SPACES, "", character + 1, " ^~");

    ErrorMessage_T* error = init_errormessage(ERR_REDEFINITION, line_number, false, value); //generate the error struct
    throw_error(handler, error); //submit the error
}

void throw_undef_error(ErrorHandler_T* handler, const char* message, unsigned int line_number, unsigned int character)
{
    const char* template = COLOR_BOLD_WHITE "%s:%d:%d " COLOR_RESET "=>" COLOR_BOLD_RED " [undef]" COLOR_RESET 
                           " %s\n %*d | %s %*s | " COLOR_BOLD_BLUE "%*shere\n" COLOR_RESET;
    char* line_code = get_line(handler->file, line_number);
    const char* pointer = "^~";

    //generate the message
    char* value = calloc(strlen(template) + strlen(handler->file->path) + strlen(message) + strlen(pointer) + strlen(line_code) + 128, sizeof(char));
    sprintf(value, template, handler->file->path, line_number + 1, character + 1, message, LINE_NUMBER_SPACES, line_number + 1, line_code, LINE_NUMBER_SPACES, "", character + 1, " ^~");

    ErrorMessage_T* error = init_errormessage(ERR_UNDEFINED, line_number, true, value); //generate the error struct
    throw_error(handler, error); //submit the error
}