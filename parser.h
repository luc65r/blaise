/**
 * @file parser.h
 * @date
 * @brief
*/

#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#include "ast.h"
#include "token.h"


typedef struct {
    Token* list;
    Token t;
    unsigned int cursor;
} Parser;

char* cpystr(char* str);

Parser* init_parser(TokenList* tlist);

void parser_advance(Parser* parser);

int parser_check_next(Parser* parser, int pos, int type);

void parse_prog_name(ASTProg* prog, Parser* parser);

void parse_var(ASTVarDecl** vars, size_t* num, Parser* parser);
 
void parse_call(ASTExpr* expr, Parser* parser);

void parse_if(ASTIfElseBlock* block, Parser* parser);

void parse_stmt(ASTStmt** stmts, size_t* num, Parser* parser);

void parse_program(ASTProg* prog, Parser* parser);

ASTProg* parse(TokenList* list);


#endif