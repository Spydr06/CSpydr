# CMake file for the CSpydr Compiler

cmake_minimum_required(VERSION 3.1.1)
project(CSpydr)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# library source
add_subdirectory(lib)
# compiler source
add_subdirectory(src/compiler)
# linter soruce
add_subdirectory(src/linter)
# compiler tests
add_subdirectory(tests/compiler)
# cspydr standard library
add_subdirectory(src/std)

# emit all warnings
if (MSVC)
    # warning level 4 and all warnings as errors
    add_compile_options(/W4 /WX)
else()
    # lots of warnings and all warnings as errors
    add_compile_options(-Wall -Wextra -pedantic -Werror)
endif()