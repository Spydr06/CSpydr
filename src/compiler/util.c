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

bool is_http_url(const char* url)
{
    return str_starts_with(url, "http://") || str_starts_with(url, "https://");
}

bool str_starts_with(const char *a, const char *b)
{
    return !strncmp(a, b, strlen(b));
}

bool str_ends_with(const char *s, const char *suffix) {
    size_t slen = strlen(s);
    size_t suffix_len = strlen(suffix);

    return suffix_len <= slen && !strcmp(s + slen - suffix_len, suffix);
}

// Round up `n` to the nearest multiple of `align`.
i64 align_to(i64 n, i64 align) 
{
    if(!align)
        align = 1;
    return (n + align - 1) / align * align;
}

char *str_replace(char *dest, const char *str1, const char *str2, const char *str3) {
    size_t i = 0, j, k = 0;
    
    // replacing substring `str2` with `str3`, assuming sufficient space
    while (str1[i] != '\0') {
        for (j = 0; str2[j] != '\0'; j++) {
            if (str1[i + j] != str2[j]) {
                break;
            }
        }
        if (str2[j] == '\0' && j > 0) {
            // we have a match: copy the replacement and skip it
            i += j;
            for (j = 0; str3[j] != '\0'; j++) {
                dest[k++] = str3[j];
            }
        } else {
            // copy the byte and skip it.
            dest[k++] = str1[i++];
        }
    }
    dest[k] = '\0';  // null terminate the destination
    return dest;
}

u64 str_count_char(const char* s, char c)
{
    u64 i = 0;
    for (i=0; s[i]; s[i]=='.' ? (void) i++ : (void) *s++);
    return i;
}