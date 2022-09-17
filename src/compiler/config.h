#ifndef CSPYDR_CONFIG_H
#define CSPYDR_CONFIG_H

//
// basic options and settings for customizing the build of CSpydr
//

#include <stdbool.h>
#include <stdlib.h>

/////////////////////////////
// Information and Socials //
/////////////////////////////

// links to me (Spydr/Spydr06/MCSpiderFe), the creator of CSpydr
// please be nice and don't change them without any reason. You may add yourself to the credits, if you changed something
#define CSPYDR_SHOW_GIT_REPOSITORY
#define CSPYDR_GIT_REPOSITORY "https://github.com/spydr06/cspydr.git"

#define CSPYDR_SHOW_GIT_DEVELOPER
#define CSPYDR_GIT_DEVELOPER  "https://github.com/spydr06"

// by default, the subreddit will not be shown in the info text
// #define CSPYDR_SHOW_SUBREDDIT
#define CSPYDR_SUBREDDIT      "https://reddit.com/r/cspydr"

///////////////////////////////
// default compiler settings //
///////////////////////////////

#if defined(__x86_64__) || defined(_M_X64)
#define CSPYDR_ARCH_X86_64 1
#elif defined(i386) || defined(__i386__) || defined(__i386) || defined(_M_IX86)
#define CSPYDR_ARCH_X86_32 1
#elif #elif defined(__aarch64__) || defined(_M_ARM64)
#define CSPYDR_ARCH_ARM64 1
#elif defined(mips) || defined(__mips__) || defined(__mips)
#define CSPYDR_ARCH_MIPS 1
#else
#define CSPYDR_ARCH_UNKNOWN 1
#endif

#if defined(__linux) || defined(__linux__)
    #define CSPYDR_LINUX 1
    #define DEFAULT_STD_PATH "/usr/share/cspydr/std"
#elif defined(_WIN32) || defined(_WIN64)
    #define CSPYDR_WINDOWS 1
    #define DEFAULT_STD_PATH "C:\\Program Files\\cspydr\\std"
#endif

#if defined(__GLIBC__)
    #define CSPYDR_LIBC_GLIBC 1
#else
    #define CSPYDR_LIBC_UNKNOWN
    #warning "Your current libc implementation is not yet supported and might lead to problems"
#endif

#define LIST_INIT_SIZE 32
#define LIST_MULTIPLIER 2
#define HASHMAP_INIT_SIZE 128
#define DEFAULT_STRING_TOKEN_SIZE 32

#define DEFAULT_COMPILE_TYPE CT_ASM

#define __CSP_MAX_FN_NUM_ARGS 128
#define __CSP_MAX_REPL_CMD_LEN (BUFSIZ * 8)
#define __CSP_DEFAULT_MAX_MACRO_CALL_DEPTH 128
#define __CSP_MAX_PASSES 32

#define MALLOC_RETRY_COUNT 10

#define OUTPUT_STREAM stdout
#define ERR_OUTPUT_STREAM stderr
#define ERR_LINE_NUMBER_SPACES 4

#define CLEAR_CODE "\e[1;1H\e[2J"

// C code generation
#define DEFAULT_CC "gcc"
#define DEFAULT_CC_FLAGS "-m64 -O3 -Wall -fPIC"

// ASM code generation
#define DEFAULT_ASSEMBLER "as"
#define DEFAULT_LINKER "ld"

#define ASM_GP_MAX 6
#define ASM_FP_MAX 8

// use __attribute__((packed))
//#define CSPYDR_PACKED_STRUCTS
#if !defined(CSPYDR_PACKED_STRUCTS) || !defined(CSPYDR_LIBC_GLIBC)
    #define __attribute__(_)
    #define __attribute(_)
#endif

const char* get_arch(void);
const char* get_os(void);
const char* get_libc(void);
void get_build(char* dest);

////////////////////////////////////////////////////
// language access to compiler config via [cfg()] //
////////////////////////////////////////////////////

typedef struct CONFIG_STRUCT {
    const char* name;
    bool (*set)(void);
} Config_T;

extern const Config_T configurations[];

#endif