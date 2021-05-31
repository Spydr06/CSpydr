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
} ErrorType_T;

typedef struct ERROR_MESSAGE
{
    ErrorType_T type;

    bool force_exit;
    char* message;
} ErrorMessage_T;

typedef struct ERROR_HANDLER
{
    SrcFile_T* file;
    List_T* error_messages;

    bool exit_after_parsing;
} ErrorHandler_T;

ErrorHandler_T* init_errorhandler();
void            free_errorhandler(ErrorHandler_T* eh);
ErrorMessage_T* init_errormessage(ErrorType_T type, unsigned int line_num, bool force_exit, char* message);
void            free_errormessage(ErrorMessage_T* em);

void throw_error(ErrorHandler_T* hander, ErrorMessage_T* message);
void throw_syntax_error(ErrorHandler_T* handler, const char* message, unsigned int line_num, unsigned int character);
void throw_redef_error(ErrorHandler_T* handler, const char* message, unsigned int line_num, unsigned int character);
void throw_undef_error(ErrorHandler_T* handler, const char* message, unsigned int line_num, unsigned int character);

#endif