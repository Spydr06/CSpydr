# CMake file for the CSpydr Compiler

cmake_minimum_required(VERSION 3.1.1)
project(CSpydr)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# emit warnings
if (MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall)
endif()

# library source
add_subdirectory(lib)
# compiler source
add_subdirectory(src/compiler)
# linter soruce
add_subdirectory(src/linter)
# api source
add_subdirectory(src/api)
# compiler tests
add_subdirectory(tests/compiler)
# cspydr standard library
add_subdirectory(src/std)

