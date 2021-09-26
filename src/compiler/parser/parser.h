#ifndef CSPYDR_PARSER_H
#define CSPYDR_PARSER_H

#include "../ast/ast.h"

void parse(ASTProg_T* ast, List_T* files, bool is_silent);

#endif