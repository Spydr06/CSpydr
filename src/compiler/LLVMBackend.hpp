#pragma once

#include <iostream>
#include "../log.h"

#include "../core/parser/AST.h"

extern "C"
{
    void compile(AST_T*, const char*, const char*);
};