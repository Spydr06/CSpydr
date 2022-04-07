# CMake file for the library source for the CSpydr Compiler

# acutest
    add_library(acutest acutest/include/acutest.h)
    target_include_directories(acutest PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
    set_target_properties(acutest PROPERTIES LINKER_LANGUAGE C)

# json-c
    find_library(json-c NAMES libjson-c json-c REQUIRED)
    find_library(bsd NAMES libbsd bsd REQUIRED)
