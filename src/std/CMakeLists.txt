# CMake file for the CSpydr Standard Library (STD)

set(CSP_STD_DESTINATION /usr/share/cspydr)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        DESTINATION ${CSP_STD_DESTINATION}
)