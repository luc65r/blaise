#include <stdio.h>
#include "lexer.h"
#include "parser.h"

int main()
{
    TokenList* res;
    
    res = lexer_scan();

    parse(res);

    return 0;
}