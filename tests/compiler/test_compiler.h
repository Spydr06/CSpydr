#include "config.h"
#include "io/log.h"
#include "platform/linux/linux_platform.h"
#include "util.h"

#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define COMPILER_TESTS {"compiler", compiler_tests}

// Should be defined by CMake; this is the defaults
#ifndef COMPILER_TEST_DIR
    #define COMPILER_TEST_DIR "tests/compiler/files"
#endif

// Should be defined by CMake; this is the defaults
#ifndef COMPILER_TEST_OUTPUT_DIR
    #define COMPILER_TEST_OUTPUT_DIR "tests/compiler/files/output"
#endif

// Should be defined by CMake; this is the default
#ifndef COMPILER_EXECUTABLE
    #define COMPILER_EXECUTABLE "bin/cspc"
#endif

i32 compiler_tests_failed = 0;
i32 compiler_tests_passed = 0;
static void test_file(const char* filename);
static i32 test_compiled_file(const char* filename, const char* output_name);

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

static void test_file(const char* filename)
{
    LOG_OK_F(COLOR_BOLD_GREEN "  * " COLOR_RESET "Testing `" COLOR_BOLD_WHITE "%s" COLOR_RESET "`\n", filename);
    fflush(OUTPUT_STREAM);

    char buf[BUFSIZ] = {};
    sprintf(buf, COMPILER_TEST_DIR "/%s", filename);

    const char output_name[] = "a.out"; 

    char* const args[] = {
        COMPILER_EXECUTABLE,
        "build",
        buf,
        "--silent",
        "-o",
        (char*) output_name,
        NULL
    };
    i32 exit_code = subprocess(COMPILER_EXECUTABLE, args, false);

    FILE* fptr = fopen(buf, "r");
    if(!fptr) 
        goto error;
    fscanf(fptr, "%[^\n]", buf);
    fclose(fptr);

    bool test_expected = strcmp(buf, "# failure") == 0 ? 1 : 0;

    if(exit_code && !test_expected)
    {
    error:
        LOG_ERROR_F("\33[2K\r\tFailed (%d)\n", exit_code);
        compiler_tests_failed++;
        return;
    }

    if(!test_expected && (exit_code = test_compiled_file(filename, output_name))) {
        goto error;
    }

    compiler_tests_passed++;
    LOG_OK("\33[2K\r\tPassed\n");
}

static i32 read_all(const char* filename, char* buffer, size_t length)
{
    FILE* fptr = fopen(filename, "rb");
    if(!fptr)
        return -1;
    fread(buffer, sizeof(char), length, fptr);
    fclose(fptr);

    return 0;
}

static i32 test_compiled_file(const char* filename, const char* output_name) 
{
    LOG_OK_F(COLOR_BOLD_GREEN "  * " COLOR_RESET "Testing `" COLOR_BOLD_WHITE "%s" COLOR_RESET "` compiled from `" COLOR_BOLD_WHITE "%s" COLOR_RESET "`\n", output_name, filename);
    fflush(OUTPUT_STREAM);

    i32 link[2];

    if(pipe(link) == -1)
        return 255;
    
    switch(fork()) {
        // error
        case -1:
            return 255;

        // fork
        case 0:
            dup2(link[1], STDOUT_FILENO);
            close(link[0]);
            close(link[1]);
            execl(output_name, output_name, NULL);
            exit(0);
            break;
        
        // parent process
        default: {
            close(link[1]);
            char output[BUFSIZ] = { 0 };
            read(link[0], output, sizeof(output));
            wait(NULL);

            char reference_file[strlen(COMPILER_TEST_OUTPUT_DIR) + strlen(filename) + 6];
            sprintf(reference_file, COMPILER_TEST_OUTPUT_DIR "/%s.out", filename);

            char reference[BUFSIZ] = { 0 };
            if(read_all(reference_file, reference, BUFSIZ) == -1) {
                fprintf(stderr, COLOR_BOLD_RED "[Error]" COLOR_RESET COLOR_RED " missing file %s\n" COLOR_RESET, reference_file);
                return 255;
            }

            if(strcmp(output, reference) != 0)
            {
                fprintf(
                    stderr, 
                    COLOR_RED "Output not matching, expect (%ld chars):\n" COLOR_RESET "%s" COLOR_RED "\ngot (%ld chars):\n" COLOR_RESET "%s\n", 
                    strlen(reference),
                    reference, 
                    strlen(output), 
                    output
                );
                return 255;
            }
        } break;
    }

    remove(output_name);

    return 0;
}