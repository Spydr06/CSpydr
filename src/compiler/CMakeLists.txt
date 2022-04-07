# CMake file for the CSpydr Compiler source
include(GNUInstallDirs)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# compiler gets treated as a library at first (without main.c)
file(GLOB_RECURSE compiler_source *.c *.cpp *.cc *.c++)
list(REMOVE_ITEM compiler_source "main.c")

add_library(libcspc ${compiler_source})
target_link_libraries(libcspc LINK_PUBLIC m json-c bsd)
set_target_properties(libcspc PROPERTIES LINKER_LANGUAGE C)

# build the main executable
add_executable(cspc main.c)
target_link_libraries(cspc libcspc)

install(PROGRAMS ${PROJECT_SOURCE_DIR}/bin/cspc 
        DESTINATION bin
        RENAME "cspc"
)