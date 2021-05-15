#ifndef CSPYDR_PLATFORM_BINDINGS_H
#define CSPYDR_PLATFORM_BINDINGS_H

#if defined(__linux__) || defined(__linux)
    #include "linux/linux_platform.h"
#elif defined(_WIN32)
    #include "win32/win32_platform.h"
#else
    #error CSpydr does not support your current platform
#endif

#endif