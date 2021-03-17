#ifndef CSPYDR_LOG_H
#define CSPYDR_LOG_H

#include <stdio.h>

#define OUTPUT_STREAM stdout

#define RESET   "\033[0m"
#define COLOR_BLACK   "\033[30m"     
#define COLOR_RED     "\033[31m"     
#define COLOR_GREEN   "\033[32m"     
#define COLOR_YELLOW  "\033[33m"     
#define COLOR_BLUE    "\033[34m"     
#define COLOR_MAGENTA "\033[35m"     
#define COLOR_CYAN    "\033[36m"     
#define COLOR_WHITE   "\033[37m"     
#define COLOR_BOLD_BLACK   "\033[1m\033[30m"     
#define COLOR_BOLD_RED     "\033[1m\033[31m"     
#define COLOR_BOLD_GREEN   "\033[1m\033[32m"     
#define COLOR_BOLD_YELLOW  "\033[1m\033[33m"     
#define COLOR_BOLD_BLUE    "\033[1m\033[34m"     
#define COLOR_BOLD_MAGENTA "\033[1m\033[35m"     
#define COLOR_BOLD_CYAN    "\033[1m\033[36m"     
#define COLOR_BOLD_WHITE   "\033[1m\033[37m"     


#define LOG_INFO(format, ...)  fprintf(OUTPUT_STREAM, COLOR_WHITE format RESET, __VA_ARGS__)
#define LOG_WARN(format, ...)  fprintf(OUTPUT_STREAM, COLOR_YELLOW format RESET, __VA_ARGS__)
#define LOG_ERROR(format, ...) fprintf(OUTPUT_STREAM, COLOR_RED format RESET, __VA_ARGS__)
#define LOG_OK(format, ...)    fprintf(OUTPUT_STREAM, COLOR_GREEN format RESET, __VA_ARGS__)

#endif