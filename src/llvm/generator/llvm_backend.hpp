#pragma once

#include "../../io/log.h"
#include "../../ast/ast.h"

extern "C" void generateLLVM(ASTRoot_T*, const char*, const char*);