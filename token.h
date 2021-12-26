/**
 * @file token.h
 * @date
 * @brief
*/

#ifndef TOKEN_H
#define TOKEN_H

#include <stdlib.h>
#include <stddef.h>
#include <gmp.h>

#include "error.h"

typedef struct {
    union {
        wchar_t *svalue;
        mpz_t nvalue;
    };

    enum {
        TOKEN_EOF = 0,
        TOKEN_ID,
        TOKEN_STRING,
        TOKEN_INT,
        TOKEN_EQ,
        TOKEN_LT,
        TOKEN_GT,
        TOKEN_COMMA,
        TOKEN_COLON,
        TOKEN_DQUOTE,
        TOKEN_ASSIGN,
        TOKEN_PROGRAM,
        TOKEN_FUNCTION,
        TOKEN_PROC,
        TOKEN_VAR,
        TOKEN_BEGIN,
        TOKEN_END,
    }type;

    Location loc;
}Token;



#endif