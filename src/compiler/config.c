#include "config.h"
#include "toolchain.h"

#include <string.h>

const char* get_arch(void)
{
#ifdef CSPYDR_ARCH_X86_64
    return "x86_64";
#elif defined(CSPYDR_ARCH_X86_32)
    return "x86_32";
#elif defined(CSPYDR_ARCH_ARM64)
    return "arm64";
#elif defined(CSPYDR_ARCH_MIPS)
    return "mips";
#else
    return "unknown";
#endif
}

const char* get_os(void)
{
#ifdef CSPYDR_LINUX
    return "linux";
#elif defined(CSPYDR_WINDOWS)
    return "windows";
#elif defined(CSPYDR_MACOS)
    return "macos";
#else
    return "unknown";
#endif
}

const char* get_libc(void)
{
#ifdef CSPYDR_LIBC_GLIBC
    return "gnu";
#else
    return "unknown";
#endif
}

void get_build(char* dest)
{
    memset(dest, 0, strlen(dest));
    strcat(dest, get_arch());
    strcat(dest, "-");
    strcat(dest, get_os());
    strcat(dest, "-");
    strcat(dest, get_libc());
}


#ifdef CSPYDR_LINUX
    static bool linux_set(Context_T* context) {
        return true;
    }
#else
    static bool linux_set(Context_T* context) {
        return false;
    }
#endif

#ifdef CSPYDR_WINDOWS
    static bool windows_set(Context_T* context) {
        return true;
    }
#else
    static bool windows_set(Context_T* context) {
        return false;
    }
#endif

#ifdef CSPYDR_MACOS
    static bool macos_set(Context_T* context) {
        return true;
    }
#else
    static bool macos_set(Context_T* context) {
        return false;
    }
#endif

static bool transpiling(Context_T* context) {
    return context->ct == CT_TRANSPILE;
}

static bool assembly(Context_T* context) {
    return context->ct == CT_ASM;
}

static bool interpreted(Context_T* context) {
    return context->ct == CT_INTERPRETER;
}

const Config_T configurations[] = {
    {"linux", linux_set},
    {"windows", windows_set},
    {"macos", macos_set},
    {"codegen_c", transpiling},
    {"codegen_asm", assembly},
    {"interpreted", interpreted},
    {NULL, NULL}
};

#undef CSPYDR_LINUX
#undef CSPYDR_WINDOWS
#undef CSPYDR_MACOS