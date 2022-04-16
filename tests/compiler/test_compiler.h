#include "config.h"
#include "io/log.h"
#include "platform/linux/linux_platform.h"
#include "util.h"

#include <stdio.h>
#include <dirent.h>
#include <string.h>

#define COMPILER_TESTS {"compiler", compiler_tests}

// Should be defined by CMake; this is the defaults
#ifndef COMPILER_TEST_DIR
    #define COMPILER_TEST_DIR "tests/compiler/files"
#endif

// Should be defined by CMake; this is the default
#ifndef COMPILER_EXECUTABLE
    #define COMPILER_EXECUTABLE "bin/cspc"
#endif

i32 compiler_tests_failed = 0;
i32 compiler_tests_passed = 0;
void test_file(char* filename);

void compiler_tests(void)
{
    printf("\n");
    compiler_tests_failed = 0;

    DIR* testdir = opendir(COMPILER_TEST_DIR);
    TEST_ASSERT(testdir != NULL);

    struct dirent* entry;
    while((entry = readdir(testdir)) != NULL)
    {
        if(entry->d_type == DT_DIR || !str_ends_with(entry->d_name, ".csp"))
            continue;
        
        test_file(entry->d_name);
    }

    closedir(testdir);

    if(compiler_tests_failed)
    {
        LOG_ERROR_F("%d out of %d compiler tests failed.\n", compiler_tests_failed, compiler_tests_failed + compiler_tests_passed);
        TEST_ASSERT(0);
        return;
    }

    LOG_OK_F("All %d compiler tests passed.\n", compiler_tests_passed);
}

void test_file(char filename[])
{
    LOG_OK_F(COLOR_BOLD_GREEN "  * " COLOR_RESET "Testing `" COLOR_BOLD_WHITE "%s" COLOR_RESET "`\n", filename);
    fflush(OUTPUT_STREAM);

    char buf[BUFSIZ] = {};
    sprintf(buf, COMPILER_TEST_DIR "/%s", filename);

    char* const args[] = {
        COMPILER_EXECUTABLE,
        "run",
        buf,
        "--silent",
        NULL
    };
    i32 exit_code = subprocess(COMPILER_EXECUTABLE, args, false);

    FILE* fptr = fopen(buf, "r");
    if(!fptr) 
        goto error;
    fscanf(fptr, "%[^\n]", buf);
    fclose(fptr);

    bool test_expected = strcmp(buf, "# success") == 0 ? 0 : strcmp(buf, "# failure") == 0 ? 1 : 0;
    bool error = (exit_code == 0) == test_expected;

    if(error)
    {
    error:
        LOG_ERROR_F("\33[2K\r\tFailed (%d)\n", exit_code);
        compiler_tests_failed++;
        return;
    }

    compiler_tests_passed++;
    LOG_OK("\33[2K\r\tPassed\n");
}