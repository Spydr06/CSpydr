# CMake file for the CSpydr Standard Library (STD)

set(CSP_STD_DESTINATION /usr/share/cspydr)

add_custom_target(stdlib ALL COMMAND /bin/bash ./generate.sh)

install(
        DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        DESTINATION ${CSP_STD_DESTINATION}
        FILES_MATCHING PATTERN "*.csp"
)