/**
 * @file lexer.h
 * @date
 * @brief
*/

#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <gmp.h>

#include "token.h"

#define MAX_LINE_SIZE 1024

typedef struct {
    FILE* fs;

    int line_nb;
    int line_cursor;

    char c;
    char line[MAX_LINE_SIZE];
}Lexer;

TokenList* lexer_scan(void);

#endif
