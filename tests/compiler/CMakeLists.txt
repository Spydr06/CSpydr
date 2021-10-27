# CMake file for CSpydr Compiler tests

include_directories(${PROJECT_SOURCE_DIR}/src/compiler)

# include the compiler as a library
add_executable(cspc-tests unit_tests.c)
target_link_libraries(cspc-tests LINK_PUBLIC libcsp)