# CMake file for the CSpydr Linter

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB_RECURSE linter_source *.c *.cpp *.cc *.c++)

add_executable(csp-lint ${linter_source})
target_link_libraries(csp-lint libcspc)
target_include_directories(csp-lint PRIVATE ${CMAKE_SOURCE_DIR}/src/compiler)

install(PROGRAMS ${PROJECT_SOURCE_DIR}/bin/csp-lint
    DESTINATION bin
    RENAME "csp-lint"
)