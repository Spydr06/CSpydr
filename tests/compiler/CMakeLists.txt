# CMake file for CSpydr Compiler tests

add_compile_definitions(COMPILER_TEST_DIR="${PROJECT_SOURCE_DIR}/tests/compiler/files")
add_compile_definitions(COMPILER_TEST_OUTPUT_DIR="${PROJECT_SOURCE_DIR}/tests/compiler/files/output")
add_compile_definitions(COMPILER_EXECUTABLE="${PROJECT_SOURCE_DIR}/bin/cspc")
include_directories(${PROJECT_SOURCE_DIR}/src/compiler)

# include the compiler as a library
add_executable(cspc-tests unit_tests.c)
target_link_libraries(cspc-tests LINK_PUBLIC libcspc acutest)