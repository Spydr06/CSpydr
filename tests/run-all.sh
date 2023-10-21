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

    function test_dir() {
        TEST_NAME=$1
        TEST_FILE=$2
        TEST_EXEC=$3

        echo "Compiling $TEST_NAME..."

        if $COMPILER_EXEC build $TEST_FILE -p $STD_DIRECTORY -o $TEST_EXEC --show-timings;
        then
            # compilation was successful, we can continue testing
            echo "Successfully compiled \`$TEST_EXEC\` from \`$TEST_FILE\`"
        else
            # compilation error
            echo "Failed to compile $TEST_NAME from $MAIN_FILE."
            exit 1
        fi

        if $TEST_EXEC;
        then
            # std tests successful
            echo "All $TEST_NAME finished successfully."
        else
            echo "Some $TEST_NAME failed."
            exit 1
        fi

        # delete the test file
        rm $TEST_EXEC
    }

    test_dir "Language Tests" "language/language_tests.csp" ./language_tests
    test_dir "Standard Library Tests" "std/std_tests.csp" ./std_tests    

    popd
else
    echo "Compiler executable \`$COMPILER_EXEC\` is missing."
    echo "Cannot perform standard library tests."
    echo "Please consider recompiling CSpydr."
    exit 1
fi
