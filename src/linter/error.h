#ifndef CSPL_ERROR_H
#define CSPL_ERROR_H

#include <stdio.h>
#include <error/error.h>

void linter_error_handler(ErrorType_T ty, Token_T* tok, const char* format, va_list args, bool is_error, const char* error_str);
void summary();
void set_error_output_file(FILE* fp);
void close_output_file();

#endif