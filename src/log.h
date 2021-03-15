#ifndef CSPYDR_LOG_H
#define CSPYDR_LOG_H

#include <stdio.h>

#define OUTPUT_STREAM stdout

#define COLOR_RED      "\x1B[31m"
#define COLOR_YELLOW   "\x1B[33m"
#define COLOR_BLUE     "\x1B[34m"
#define COLOR_GREEN    "\x1B[32m"
#define COLOR_WHITE    "\x1B[37m"
#define RESET          "\x1B[0m"

#define LOG_INFO(format, ...)  fprintf(OUTPUT_STREAM, COLOR_WHITE format RESET, __VA_ARGS__)
#define LOG_WARN(format, ...)  fprintf(OUTPUT_STREAM, COLOR_YELLOW format RESET, __VA_ARGS__)
#define LOG_ERROR(format, ...) fprintf(OUTPUT_STREAM, COLOR_RED format RESET, __VA_ARGS__)
#define LOG_OK(format, ...)    fprintf(OUTPUT_STREAM, COLOR_GREEN format RESET, __VA_ARGS__)

#endif