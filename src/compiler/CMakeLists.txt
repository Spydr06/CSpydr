# CMake file for the CSpydr Compiler source

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# compiler gets treated as a library at first (without main.c)
file(GLOB_RECURSE compiler_source *.c)
list(REMOVE_ITEM compiler_source "main.c")

add_library(cspc ${compiler_source})
target_link_libraries(cspc LINK_PUBLIC LittleXML c-vector llvm-c)
set_target_properties(cspc PROPERTIES LINKER_LANGUAGE C)

# build the main executable
add_executable(cspydr main.c)
target_link_libraries(cspydr cspc)
