#include "util.h"

#include <stdbool.h>
#include <string.h>

#ifndef __GLIBC__
char *strsep(char **stringp, const char *delim) 
{
    if (*stringp == NULL) { return NULL; }
    char *token_start = *stringp;
    *stringp = strpbrk(token_start, delim);
    if (*stringp) {
        **stringp = '\0';
        (*stringp)++;
    }
    return token_start;
}
#endif

bool str_starts_with(const char *a, const char *b)
{
    return !strncmp(a, b, strlen(b));
}

// Round up `n` to the nearest multiple of `align`.
i64 align_to(i64 n, i64 align) 
{
    if(!align)
        align = 1;
    return (n + align - 1) / align * align;
}