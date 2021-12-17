# CMake file for the CSpydr Compiler source

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# compiler gets treated as a library at first (without main.c)
file(GLOB_RECURSE compiler_source *.c)
list(REMOVE_ITEM compiler_source "main.c")

add_library(libcsp ${compiler_source})
target_link_libraries(libcsp LINK_PUBLIC LittleXML c-vector llvm)
set_target_properties(libcsp PROPERTIES LINKER_LANGUAGE C)

# build the main executable
add_executable(cspc main.c)
target_link_libraries(cspc libcsp)

install(PROGRAMS ${PROJECT_SOURCE_DIR}/bin/cspc 
        DESTINATION bin
        RENAME "cspc"
)