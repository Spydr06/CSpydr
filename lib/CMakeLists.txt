# CMake file for the library source for the CSpydr Compiler

# acutest
    add_library(acutest acutest/include/acutest.h)
    target_include_directories(acutest PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
    set_target_properties(acutest PROPERTIES LINKER_LANGUAGE C)

# json-c
    if(NOT EXISTS lib/json-c)
        message("lib/json-c not found, cloning repository...")
        execute_process(COMMAND git clone https://github.com/json-c/json-c.git lib/json-c)
    else()
        message("lib/json-c found.")
    endif()
    set(BUILD_SHARED_LIBS FALSE)
    add_subdirectory(json-c)

# dirent for windows
    if(MSVC)
        if(NOT EXISTS lib/dirent)
            message("lib/dirent not found, cloning repository...")
            execute_process(COMMAND git clone https://github.com/tronkko/dirent.git lib/dirent)
        else()
            message("lib/dirent found.")
        endif() 
        add_subdirectory(dirent)
    else()
        set(dirent "")
    endif()
