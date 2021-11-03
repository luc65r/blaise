%{
#include <assert.h>
#include <gmp.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unictype.h>
#include <uninorm.h>
#include <unistr.h>

#include "ast.h"
#include "vec.h"
#include "parse.h"

#define YY_USER_ACTION                          \
    yylloc.ec += yyleng;

#define UPDATE_LOCATION() do {                  \
        yylloc.sl = yylloc.el;                  \
        yylloc.sc = yylloc.ec;                  \
    } while (0)                                 \

void yyerror(const char *s) {
    fprintf(stderr, "error: %s\n", s);
}

bool is_ident(const uint8_t *s) {
    ucs4_t c;
    s = u8_next(&c, s);
    if (!uc_is_property_xid_start(c))
        return false;
    for (; s != NULL; s = u8_next(&c, s))
        if (!uc_is_property_xid_continue(c))
            return false;
    return true;
}
%}

%option 8bit noyywrap noinput nounput never-interactive nounistd
%option warn

A [aA]
B [bB]
C [cC]
D [dD]
E [eE]
F [fF]
G [gG]
H [hH]
I [iI]
J [jJ]
K [kK]
L [lL]
M [mM]
N [nN]
O [oO]
P [pP]
Q [qQ]
R [rR]
S [sS]
T [tT]
U [uU]
V [vV]
W [wW]
X [xX]
Y [yY]
Z [zZ]
EACUTE ([eE]|\xc3[\x89\xa9])
AGRAVE ([aA]|\xc3[\xa0\x80])

%%

%{
UPDATE_LOCATION();
%}

{P}{R}{O}{G}{R}{A}{M}{M}{E} return TOKEN_PROGRAM;
{D}{EACUTE}{B}{U}{T} return TOKEN_BEGIN;
{F}{I}{N} return TOKEN_END;
{F}{O}{N}{C}{T}{I}{O}{N} return TOKEN_FUNCTION;
{P}{R}{O}{C}{EACUTE}{D}{U}{R}{E} return TOKEN_PROCEDURE;
{V}{A}{R}{I}{A}{B}{L}{E}{S} return TOKEN_VARIABLES;
{I}{F} return TOKEN_IF;
{A}{L}{O}{R}{S} return TOKEN_THEN;
{S}{I}{N}{O}{N} return TOKEN_ELSE;
{P}{O}{U}{R} return TOKEN_FOR;
{D}{E} return TOKEN_FROM;
{AGRAVE} return TOKEN_TO;
{F}{A}{I}{R}{E} return TOKEN_DO;
{T}{A}{N}{T} return TOKEN_WHILE;
{Q}{U}{E} return TOKEN_QUE;
{R}{EACUTE}{P}{EACUTE}{T}{E}{R} return TOKEN_REPEAT;
{V}{R}{A}{I} return TOKEN_TRUE;
{F}{A}{U}{X} return TOKEN_FALSE;
"<-"|"←" return TOKEN_ASSIGNMENT;
"(" return TOKEN_LPAREN;
")" return TOKEN_RPAREN;
"," return TOKEN_COMMA;
":" return TOKEN_COLON;
"+" return TOKEN_PLUS;
"-" return TOKEN_MINUS;
"*" return TOKEN_STAR;
{D}{I}{V} return TOKEN_DIV;
{M}{O}{D} return TOKEN_MOD;
"=="|"=" return TOKEN_EQ;
"!=" return TOKEN_NEQ;
"<=" return TOKEN_LE;
"<" return TOKEN_LT;
">=" return TOKEN_GE;
">" return TOKEN_GT;
"//"[^\n]*\n
"\""("\\\""|[^"])*"\"" {
    yylval.str = strdup(yytext + 1);
    yylval.str[strlen(yylval.str) - 1] = '\0';
    return TOKEN_STRLIT;
}
[0-9]+ {
    int err = mpz_init_set_str(yylval.integer, yytext, 10);
    assert(err == 0);
    return TOKEN_INTLIT;
}
"\n" {
    yylloc.el++;
    yylloc.ec = 1;
    continue;
}
[ \t]+ { /* TODO: unicode white-space */
    UPDATE_LOCATION();
    continue;
}
[^ \t\n]+ {
    const uint8_t *s = (uint8_t *)yytext;
    if (!is_ident(s))
        REJECT;
    size_t len;
    char *str = (char *)u8_normalize(UNINORM_NFKC, s, u8_strlen(s), NULL, &len);
    assert(str != NULL);
    str = realloc(str, len + 1);
    assert(str != NULL);
    str[len] = '\0';
    yylval.ident = str;
    return TOKEN_IDENT;
}
. fprintf(stderr, "Unrecognized character: %s\n", yytext); yyterminate();

%%
