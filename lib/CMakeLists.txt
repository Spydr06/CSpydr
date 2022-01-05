# CMake file for the library source for the CSpydr Compiler

# acutest
    add_library(acutest acutest/include/acutest.h)
    target_include_directories(acutest PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
    set_target_properties(acutest PROPERTIES LINKER_LANGUAGE C)

# c-vector
    add_library(c-vector c-vector/vec.c)
    target_include_directories(c-vector PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
    set_target_properties(c-vector PROPERTIES LINKER_LANGUAGE C)
# LittleXML
    add_library(LittleXML LittleXML/lxml.h)
    target_include_directories(LittleXML PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
    set_target_properties(LittleXML PROPERTIES LINKER_LANGUAGE C)

# json-c
    find_package(json-c CONFIG)

# llvm-c
    add_subdirectory(llvm)

