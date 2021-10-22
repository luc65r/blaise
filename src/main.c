#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>

#include "parse.h"
#include "scan.h"

int main() {
    yyin = stdin;
    yyparse();

    return 0;
}
