/**
 * @file token.c
 * @date
 * @brief
*/
#include "token.h"

Token* init_token(int type, wchar_t *svalue, mpz_t nvalue)
{
    Token* token = calloc(1, sizeof(Token));
    token->type = type;

    if (type == TOKEN_ID || type == TOKEN_STRING)
        token->svalue = svalue;
    else if (type == TOKEN_INT)
        mpz_set(token->nvalue, nvalue);
}