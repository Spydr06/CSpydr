# CMake file for the CSpydr Compiler source
include(GNUInstallDirs)
include(CheckIncludeFile)
include(CheckIncludeFiles)
find_package(PkgConfig REQUIRED)

pkg_check_modules(LIBPKGCONF REQUIRED libpkgconf)
pkg_check_modules(JSON-C REQUIRED json-c)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

find_package(Git)

file(WRITE ${CMAKE_CURRENT_SOURCE_DIR}/version.cmake
"execute_process(
    COMMAND ${GIT_EXECUTABLE} describe --always --dirty
    OUTPUT_VARIABLE VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
configure_file(\${SRC} \${DST} @ONLY)
")

add_custom_target(
    version
    ${CMAKE_COMMAND} -DSRC=${CMAKE_CURRENT_SOURCE_DIR}/version.h.in
                     -DDST=${CMAKE_CURRENT_SOURCE_DIR}/version.h
                     -P ${CMAKE_CURRENT_SOURCE_DIR}/version.cmake    
)

# compiler gets treated as a library at first (without main.c)
file(GLOB_RECURSE compiler_source *.c *.cpp *.cc *.c++)
list(REMOVE_ITEM compiler_source "main.c")

add_library(libcspc STATIC ${compiler_source})
set_target_properties(libcspc PROPERTIES 
        LINKER_LANGUAGE C
        POSITION_INDEPENDENT_CODE ON
)

target_compile_options(libcspc PUBLIC ${LIBPKGCONF_CFLAGS_OTHER} ${JSON-C_CFLAGS_OTHER})
target_include_directories(libcspc PUBLIC ${LIBPKGCONF_INCLUDE_DIRS} ${JSON-C_INCLUDE_DIRS})

add_dependencies(libcspc version)

if(${NO_LLVM})
        add_compile_definitions(CSPYDR_NO_LLVM)
        target_link_libraries(libcspc LINK_PUBLIC m ${JSON-C_LIBRARIES} ${LIBPKGCONF_LIBRARIES})
else()
        add_compile_definitions(CSPYDR_USE_LLVM)
        
        find_package(LLVM REQUIRED CONFIG)
        message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")

        execute_process(COMMAND llvm-config --libs all OUTPUT_STRIP_TRAILING_WHITESPACE OUTPUT_VARIABLE llvm_libraries)
        execute_process(COMMAND llvm-config --libdir OUTPUT_STRIP_TRAILING_WHITESPACE OUTPUT_VARIABLE llvm_link_dirs)
        execute_process(COMMAND llvm-config --includedir OUTPUT_STRIP_TRAILING_WHITESPACE OUTPUT_VARIABLE llvm_include_dirs)
       
        target_include_directories(libcspc PUBLIC "${llvm_include_dirs}")
        target_link_directories(libcspc PUBLIC "${llvm_link_dirs}")
        target_link_libraries(libcspc LINK_PUBLIC m ${JSON-C_LIBRARIES} ${LIBPKGCONF_LIBRARIES} "${llvm_libraries}")
endif()

# build the main executable
add_executable(cspc main.c)
target_link_libraries(cspc libcspc)

install(PROGRAMS ${PROJECT_SOURCE_DIR}/bin/cspc 
        DESTINATION bin
        RENAME "cspc"
)
