/*
    LIBCSPC API HEADERS
    
    Copyright (c) 2021 - 2022 Spydr06
    This code and all code of CSpydr is licensed under the MIT license.
    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    The software is provided "as is", without warranty of any kind.

    cspydr.h features an API for cspydr and its compiler cspc for C/C++
*/

#ifndef __CSPYDR_H
#define __CSPYDR_H

#ifdef __cplusplus
namespace cspydr {
    extern "C" {
#endif

typedef struct CSPYDR_COMPILER_STRUCT Compiler_T;
typedef enum {
    COMPILER_NONE = 0,
    COMPILER_INIT,
    COMPILER_PARSED,
    COMPILER_OPTIMIZED,
    COMPILER_GENERATED,
    COMPILER_EXECUTED
} CompilerStatus_T;

extern Compiler_T* csp_init_compiler();
extern void csp_free_compiler(Compiler_T* compiler);
extern CompilerStatus_T csp_get_status(Compiler_T* compiler);
const char* csp_status_str(CompilerStatus_T status);

#ifdef __cplusplus
    } // extern "C"
} // namespace cspydr
#endif

#endif // __CSPYDR_H