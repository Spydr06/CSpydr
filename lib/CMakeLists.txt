# CMake file for the library source for the CSpydr Compiler

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
    set(BUILD_SHARED_LIBS FALSE)
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
