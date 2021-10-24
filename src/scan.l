%{
#include <stdio.h>
#include <string.h>

#include "ast.h"
#include "vec.h"
#include "parse.h"
%}

%option 8bit noyywrap never-interactive nounistd
%option warn

%%

[ \t\n]+
PROGRAMME return TOKEN_PROGRAM;
DEBUT return TOKEN_BEGIN;
FIN return TOKEN_END;
FONCTION return TOKEN_FUNCTION;
PROCEDURE return TOKEN_PROCEDURE;
VARIABLES return TOKEN_VARIABLES;
[a-zA-Z_][a-zA-Z0-9_]* yylval.ident = strdup(yytext); return TOKEN_IDENTIFIER;
"<-" return TOKEN_ASSIGNMENT;
"(" return TOKEN_LPAREN;
")" return TOKEN_RPAREN;
"," return TOKEN_COMMA;
":" return TOKEN_COLON;
"+" return TOKEN_PLUS;
. fprintf(stderr, "Unrecognized character: %s\n", yytext); yyterminate();

%%
