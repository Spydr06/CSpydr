# CMake file for the library source for the CSpydr Compiler

include(CheckIncludeFiles)

# acutest
add_library(acutest acutest/include/acutest.h)
target_include_directories(acutest PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
set_target_properties(acutest PROPERTIES LINKER_LANGUAGE C)

# json-c
if(NOT EXISTS ${CMAKE_CURRENT_LIST_DIR}/json-c OR NOT EXISTS ${CMAKE_CURRENT_LIST_DIR}/json-c/CMakeLists.txt)
    message("lib/json-c not found, cloning repository...")
    execute_process(COMMAND git clone https://github.com/json-c/json-c.git ${CMAKE_CURRENT_LIST_DIR}/json-c)
else()
    message("lib/json-c found.")
endif()
set(BUILD_SHARED_LIBS OFF)
add_subdirectory(json-c)

# dirent for windows
if(MSVC)
    if(NOT EXISTS ${CMAKE_CURRENT_LIST_DIR}/dirent OR NOT EXISTS ${CMAKE_CURRENT_LIST_DIR}/dirent/CMakeLists.txt)
        message("lib/dirent not found, cloning repository...")
        execute_process(COMMAND git clone https://github.com/tronkko/dirent.git ${CMAKE_CURRENT_LIST_DIR}/dirent)
    else()
        message("lib/dirent found.")
    endif() 
    add_subdirectory(dirent)
else()
    set(dirent "")
endif()

# llvm
find_package(LLVM)
if(${LLVM_FOUND} AND NOT NO_LLVM)
    message(STATUS "Looking for llvm - found")
    check_include_files(
        "llvm-c/Analysis.h;llvm-c/Core.h;llvm-c/TargetMachine.h;llvm-c/ErrorHandling.h;llvm-c/Types.h;llvm-c/Transforms/Scalar.h;llvm-c/Target.h;llvm-c/ExecutionEngine.h;llvm-c/BitWriter.h;llvm-c/Linker.h"
        HAVE_LLVM_C
    )
    if(NOT HAVE_LLVM_C)
        message(SEND_ERROR "-- Looking for llvm-c bindings - not found")
    endif()
else()
    message(STATUS "Looking for llvm - not found")
    set(NO_LLVM TRUE)
endif()