#ifndef CSPYDR_LOG_H
#define CSPYDR_LOG_H

#include <stdio.h>
#include "config.h"

#define COLOR_RESET   "\033[0m"
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

#define LOG_INFO_F(format, ...)   fprintf(OUTPUT_STREAM, COLOR_WHITE format COLOR_RESET, __VA_ARGS__)
#define LOG_WARN_F(format, ...)   fprintf(OUTPUT_STREAM, COLOR_YELLOW format COLOR_RESET, __VA_ARGS__)
#define LOG_ERROR_F(format, ...)  fprintf(OUTPUT_STREAM, COLOR_RED format COLOR_RESET, __VA_ARGS__)
#define LOG_OK_F(format, ...)     fprintf(OUTPUT_STREAM, COLOR_GREEN format COLOR_RESET, __VA_ARGS__)

#define LOG_INFO(format)   fprintf(OUTPUT_STREAM, COLOR_WHITE format COLOR_RESET)
#define LOG_WARN(format)   fprintf(OUTPUT_STREAM, COLOR_YELLOW format COLOR_RESET)
#define LOG_ERROR(format)  fprintf(OUTPUT_STREAM, COLOR_RED format COLOR_RESET)
#define LOG_OK(format)     fprintf(OUTPUT_STREAM, COLOR_GREEN format COLOR_RESET)

#ifdef _WIN32
    #include <conio.h>
    #define LOG_CLEAR() clrscr()
#else
    #define LOG_CLEAR() system("clear")
#endif

#endif