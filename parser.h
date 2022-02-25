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

ASTProg* parse(TokenList* list);

#endif