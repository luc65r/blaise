/**
 * @file token.h
 * @date
 * @brief
*/

#ifndef TOKEN_H
#define TOKEN_H

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>

typedef struct {
    enum {
        TOKEN_ID = 0,
        TOKEN_STRING,
        TOKEN_INT,
        TOKEN_SLASH,
        TOKEN_STAR,
        TOKEN_PLUS,
        TOKEN_MINUS,
        TOKEN_EQ,
        TOKEN_LT,
        TOKEN_GT,
        TOKEN_COMMA,
        TOKEN_COLON,
        TOKEN_LPAREN,
        TOKEN_RPAREN,
        TOKEN_LBRACKET,
        TOKEN_RBRACKET,
        TOKEN_LSQBRACKET,
        TOKEN_RSQBRACKET,
        TOKEN_ASSIGN,
        TOKEN_PROGRAM,
        TOKEN_FUNCTION,
        TOKEN_PROC,
        TOKEN_VAR,
        TOKEN_BEGIN,
        TOKEN_END,
    }type;

    char* str;
}Token;

typedef struct
{
    int cursor;
    int capacity;
    Token* list;
}TokenList;

TokenList* init_token_list();

void push_token(TokenList* chunk, int type);

void print_token_list(TokenList* chunk);

void pop_token_list(TokenList* chunk);

#endif