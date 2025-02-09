#ifndef CSPYDR_PARSER_DIRECTIVES_H
#define CSPYDR_PARSER_DIRECTIVES_H

#include "list.h"
#include "parser/parser.h"

void parse_directives(Parser_T* parser, List_T* objects, bool in_extern_block, bool is_extern_c);

#endif
