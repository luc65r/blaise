/**
 * @file lexer.h
 * @date
 * @brief
*/

#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "token.h"

#define LINE_MAX 1024

typedef struct {
    FILE* fs;

    int line_nb;
    int line_cursor;

    char c;
    char line[LINE_MAX];
    char utf_line[LINE_MAX];
}Lexer;

TokenList* lexer_scan(void);


#endif
