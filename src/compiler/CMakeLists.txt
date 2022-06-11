# CMake file for the CSpydr Compiler source
include(GNUInstallDirs)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# compiler gets treated as a library at first (without main.c)
file(GLOB_RECURSE compiler_source *.c *.cpp *.cc *.c++)
list(REMOVE_ITEM compiler_source "main.c")

add_library(libcspc ${compiler_source})
set_target_properties(libcspc PROPERTIES 
        LINKER_LANGUAGE C
        POSITION_INDEPENDENT_CODE ON
)

if(${NO_LLVM})
        add_compile_definitions(CSPYDR_NO_LLVM)
        target_link_libraries(libcspc LINK_PUBLIC m json-c bsd)
else()
        add_compile_definitions(CSPYDR_USE_LLVM)
        
        find_package(LLVM REQUIRED CONFIG)
        message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")

        execute_process(COMMAND llvm-config --libs all OUTPUT_STRIP_TRAILING_WHITESPACE OUTPUT_VARIABLE llvm_libraries)
        target_include_directories(libcspc INTERFACE ${LLVM_INCLUDE_DIRS})
        target_link_libraries(libcspc LINK_PUBLIC m json-c bsd "${llvm_libraries}")
endif()

# build the main executable
add_executable(cspc main.c)
target_link_libraries(cspc libcspc)


install(PROGRAMS ${PROJECT_SOURCE_DIR}/bin/cspc 
        DESTINATION bin
        RENAME "cspc"
)