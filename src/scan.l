%{
#include "ast.h"
#include "vec.h"
#include "parse.h"
%}

%option 8bit noyywrap never-interactive nounistd
%option warn

%%

[ \t\n] ;
PROGRAMME return TOKEN_PROGRAM;
DEBUT return TOKEN_BEGIN;
FIN return TOKEN_END;
FONCTION return TOKEN_FUNCTION;
PROCEDURE return TOKEN_PROCEDURE;
VARIABLES return TOKEN_VARIABLES;
"<-" return TOKEN_ASSIGNMENT;
"(" return TOKEN_LPAREN;
")" return TOKEN_RPAREN;
"," return TOKEN_COMMA;
":" return TOKEN_COLON;
"+" return TOKEN_PLUS;

[a-zA-Z][a-zA-Z0-9]* yylval.ident = yytext; return TOKEN_IDENTIFIER;

%%
