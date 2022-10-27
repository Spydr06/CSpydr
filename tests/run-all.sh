#!/bin/bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

# compiler tests directly testing the compiler from C

COMPILER_TEST_EXEC="$SCRIPT_DIR/../bin/cspc-tests"

if [ -f "$COMPILER_TEST_EXEC" ];
then
    if $COMPILER_TEST_EXEC; then
        echo "Compiler tests finished successfully."
    else
        echo "Not all compiler tests finished successfully."
    fi
else
    echo "Compiler test executable \`$COMPILER_TEST_EXEC\` is missing."
    echo "Cannot perform compiler tests."
    echo "Please consider recompiling CSpydr."
    exit 1
fi

# CSpydr standard library tests

STD_DIRECTORY="$SCRIPT_DIR/../src/std/"

if [ ! -d "$STD_DIRECTORY" ];
then
    echo "Standard library \`$STD_DIRECTORY\` is missing."
    echo "Cannot perform standard library tests."
    echo "Please consider recompiling CSpydr."
    exit 1
fi

COMPILER_EXEC="$SCRIPT_DIR/../bin/cspc"
 
if [ -f "$COMPILER_EXEC" ];
then
    pushd $SCRIPT_DIR

    TEST_EXEC="./std-tests.out" 
    MAIN_FILE="./std/std_tests.csp"

    # compile tests
    echo "Compiling tests..."

    if $COMPILER_EXEC build $MAIN_FILE -p $STD_DIRECTORY -o $TEST_EXEC --show-timings;
    then
        # compilation was successful, we can continue testing
        echo "Successfully compiled \`$TEST_EXEC\` from \`$MAIN_FILE\`"
    else
        # compilation error
        echo "Failed to compile standard library tests from $MAIN_FILE."
        exit 1
    fi

    if $TEST_EXEC;
    then
        # std tests successful
        echo "All standard library tests finished successfully."
    else
        echo "Some standard library tests failed."
        exit 1
    fi

    # delete the test file
    rm $TEST_EXEC

    popd
else
    echo "Compiler executable \`$COMPILER_EXEC\` is missing."
    echo "Cannot perform standard library tests."
    echo "Please consider recompiling CSpydr."
    exit 1
fi
