# CMake file for the CSpydr API
include(GNUInstallDirs)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
file(GLOB api_source *.c *.cpp *.cc *.c++)

add_library(cspydr SHARED ${api_source})

file(GLOB_RECURSE public_headers include/*.h include/*.hpp)

set_target_properties(cspydr PROPERTIES
    SOVERSION 1
    PUBLIC_HEADER "${public_headers}"
    POSITION_INDEPENDENT_CODE ON
)
target_include_directories(cspydr PRIVATE ${CMAKE_SOURCE_DIR}/src/compiler)
target_link_libraries(cspydr libcspc)

install(TARGETS cspydr
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# pkg-config
configure_file(cspydr.pc.in cspydr.pc @ONLY)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/cspydr.pc
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig
)