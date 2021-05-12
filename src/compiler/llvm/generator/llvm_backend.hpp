#pragma once

#include "../../io/log.h"
#include "../../ast/ast.h"

extern "C" void generateLLVM(ASTProgram_T*, const char*, const char*);