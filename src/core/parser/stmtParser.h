#ifndef CSPYDR_STMT_PARSER_H
#define CSPYDR_STMT_PARSER_H

#include "AST.h"
#include "parser.h"

ASTIfStmt_T* parserParseIf(parser_T* parser);
ASTForStmt_T* parserParseFor(parser_T* parser);
ASTWhileStmt_T* parserParseWhile(parser_T* parser);
ASTReturnStmt_T* parserParseReturnStmt(parser_T* parser);
ASTExitStmt_T* parserParseExitStmt(parser_T* parser);

#endif