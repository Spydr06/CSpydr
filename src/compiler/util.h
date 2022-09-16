#ifndef CSPYDR_TYPES_H
#define CSPYDR_TYPES_H

#include <stdint.h>
#include <stdbool.h>

#include "config.h"

#define LEN(arr) (sizeof(arr) / sizeof(*arr))

/*
 * Type definitions
 */

// signed integers
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

// unsigned integers
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64; 

// floats
typedef float f32;
typedef double f64;
typedef long double f80;

#ifndef CSPYDR_GNU_LIBC
char *strsep(char **stringp, const char *delim);
#endif

bool is_http_url(const char* url);
bool str_starts_with(const char *a, const char *b);
bool str_ends_with(const char *s, const char *suffix);
i64 align_to(i64 n, i64 align);
char *str_replace(char *dest, const char *str1, const char *str2, const char *str3);
u64 str_count_char(const char* s, char c);

#endif
