#include "config.h"
#include "toolchain.h"

#include <globals.h>

#ifdef CSPYDR_LINUX
    static bool linux_set(void) {
        return true;
    }
#else
    static bool linux_set(void) {
        return false;
    }
#endif

#ifdef CSPYDR_WINDOWS
    static bool windows_set(void) {
        return true;
    }
#else
    static bool windows_set(void) {
        return false;
    }
#endif

#ifdef CSPYDR_MACOS
    static bool macos_set(void) {
        return true;
    }
#else
    static bool macos_set(void) {
        return false;
    }
#endif

static bool transpiling(void) {
    return global.ct == CT_TRANSPILE;
}

static bool assembly(void) {
    return global.ct == CT_ASM;
}

const Config_T configurations[] = {
    {"linux", linux_set},
    {"windows", windows_set},
    {"macos", macos_set},
    {"codegen_c", transpiling},
    {"codegen_asm", assembly},
    {NULL, NULL}
};

#undef CSPYDR_LINUX
#undef CSPYDR_WINDOWS
#undef CSPYDR_MACOS