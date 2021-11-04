#define _GNU_SOURCE

#include <jansson.h>
#include <stdio.h>
#include <stdlib.h>

#include "ast.h"
#include "eval.h"
#include "vec.h"

#include "parse.h"
#include "scan.h"

extern ASTProg *ast;

int main(void) {
    yyin = stdin;
    yyparse();

    json_t *j = ast_json(ast);
    //eval(ast);

    json_dumpf(j, stdout, JSON_INDENT(2));
    puts("");

    return 0;
}
