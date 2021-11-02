#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>

#include "ast.h"
#include "eval.h"
#include "vec.h"

#include "parse.h"
#include "scan.h"

extern ASTProgram *ast;

int main(void) {
    yyin = stdin;
    yyparse();

    //ast_dump(ast);
    eval(ast);

    return 0;
}
