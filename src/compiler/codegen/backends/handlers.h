#ifdef HEADER_DEFS_ONLY

#ifndef CSPYDR_BACKEND_HANDLERS_H
#define CSPYDR_BACKEND_HANDLERS_H

// Normal header stuff

extern const BackendCallbacks_T C99_CALLBACKS;
extern const BackendCallbacks_T X86_64_GAS_CALLBACKS;

#endif

#else

// Backend handler entries

{
    "c99",
    ARCH_ANY,
    PLATFORM_ANY,
    true,
    &C99_CALLBACKS,
},
{
    "x86_64-gas",
    ARCH_X86_64,
    PLATFORM_LINUX,
    true,
    &X86_64_GAS_CALLBACKS,
},

#endif

