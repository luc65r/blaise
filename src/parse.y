%{
#include <stdio.h>

extern int yylex();
void yyerror(const char *s) {
    fprintf(stderr, "ERROR: %s\n", s);
}
%}

%token PROGRAMME DEBUT FIN IDENTIFIER

%%

file : PROGRAMME identifier DEBUT FIN

identifier : IDENTIFIER

%%
