#ifndef CSPYDR_CONFIG_H
#define CSPYDR_CONFIG_H

//
// basic options and settings for customizing the build of CSpydr
//

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

#if defined(__linux) || defined(__linux__)
    #define CSPYDR_LINUX 1
#else
    #error "Your current platform is not supported by CSpydr"
#endif
#if defined(__gnu_linux__) || defined(__GNUC__)
    #define CSPYDR_GNU_LIBC 1
#else
    #warn "Your current libc implementation is not yet supported and might lead to problems"
#endif

#define DEFAULT_COMPILE_TYPE CT_ASM

#define __CSP_MAX_TOKEN_SIZE 128
#define __CSP_MAX_FN_NUM_ARGS 128
#define __CSP_MAX_REPL_CMD_LEN (BUFSIZ * 8)

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
#define CSPYDR_PACKED_STRUCTS

#endif