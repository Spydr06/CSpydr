#ifndef CSPYDR_EXCEPTION_C
#define CSPYDR_EXCEPTION_C

#include <setjmp.h>

typedef jmp_buf Exception_T;

#define try(exception)     \
    if(!setjmp(exception))

#define catch else

#define throw(exception)  \
    do longjmp(exception, 1); while(0)

#endif