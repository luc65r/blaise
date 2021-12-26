/**
 * @file lexer.c
 * @date
 * @brief
*/
#include "lexer.h"

Lexer* init_lexer_from_file()
{
    Lexer* lexer = calloc(1, sizeof(Lexer));

    setlocale(LC_ALL, "en_US.UTF-8");

    lexer->lineNb = 0;
    lexer->colNb = 0;



    return lexer;
}

