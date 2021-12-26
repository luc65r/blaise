/**
 * @file lexer.h
 * @date
 * @brief
*/

#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>
#include <stddef.h>
#include <locale.h>

typedef struct {
    FILE* fs;

    size_t lineNb;
    size_t colNb;

    char c;
    char line[1024];
    wchar_t rawLine[1024];
}Lexer;

Lexer* init_lexer_from_file();


#endif

