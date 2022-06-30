# CMake file for the library source for the CSpydr Compiler

# acutest
    add_library(acutest acutest/include/acutest.h)
    target_include_directories(acutest PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
    set_target_properties(acutest PROPERTIES LINKER_LANGUAGE C)

# json-c
    set(BUILD_SHARED_LIBS FALSE)
    add_subdirectory(json-c)

# dirent for windows
    if(MSVC)
        add_subdirectory(dirent)
    else()
        set(dirent "")
    endif()
