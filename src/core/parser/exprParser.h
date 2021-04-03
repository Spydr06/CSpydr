#include "AST.h"
#ifndef CSPYDR_EXPR_PARSER_H
#define CSPYDR_EXPR_PARSER_H

#include "parser.h"
#include <stdbool.h>

ASTExpr_T* parserParseExpr(parser_T* parser);
ASTAssignment_T* parserParseAssinment(parser_T* parser, char* targetCallee);
ASTExprFnCall_T* parserParseFunctionCall(parser_T* parser, char* callee);
ASTLocal_T* parserParseLocal(parser_T* parser, bool includeLet);
#endif