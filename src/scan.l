%{
#include "parse.h"
%}

%option 8bit noyywrap never-interactive nounistd
%option bison-bridge

%%

PROGRAMME return PROGRAMME;
DEBUT return DEBUT;
FIN return FIN;

[a-zA-Z][a-zA-Z0-9]* return IDENTIFIER;

%%
