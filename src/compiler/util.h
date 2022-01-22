#ifndef CSPYDR_TYPES_H
#define CSPYDR_TYPES_H

#include <stdint.h>
#include <stdbool.h>

/*
 * Type definitions
 */

// signed integers
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

#ifdef __GNUC__
typedef signed __int128 i128;
#else
typedef signed long long i128;
#endif

// unsigned integers
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64; 

#ifdef __GNUC__
typedef unsigned __int128 u128;
#else
typedef unsigned long long u128;
#endif

// floats
typedef float f32;
typedef double f64;
typedef long double f80;

#ifndef __GNUC__
char *strsep(char **stringp, const char *delim);
#endif

bool str_starts_with(const char *a, const char *b);

#endif