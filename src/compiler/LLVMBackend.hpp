#pragma once

#include <iostream>
#include "../log.h"

#include "../core/AST.h"

extern "C"
{
    void compile(AST_T*, const char*);
};