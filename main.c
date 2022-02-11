#include <stdio.h>
#include "lexer.h"

int main()
{
    TokenList* res;
    
    res = lexer_scan();

    return 0;
}