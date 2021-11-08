# CMake file for the CSpydr Compiler

cmake_minimum_required(VERSION 3.0.0)
project(CSpydr)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# compiler source
add_subdirectory(src/compiler)
# library source
add_subdirectory(lib)
# compiler tests
add_subdirectory(tests/compiler)
# cspydr standard library
add_subdirectory(src/std)

# emit all warnings
if(CMAKE_COMPILER_IS_GNUCC)
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wall -Wextra")
endif()
if(MSVC)
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} /W4")
endif()