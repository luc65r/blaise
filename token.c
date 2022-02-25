/**
 * @file token.c
 * @date
 * @brief
*/
#include "token.h"

TokenList* init_token_list() 
{
    TokenList* res = calloc(1, sizeof(TokenList));
    res->list = (Token*) calloc(32, sizeof(Token));
    res->cursor = 0;
    res->capacity = 32;

    return res;
}

void push_token(TokenList* chunk, int type, Loc loc)
{
    if (chunk->cursor == chunk->capacity){
        chunk->capacity += 16;
        chunk->list = realloc(chunk->list, chunk->capacity * sizeof(Token));
    }

    chunk->list[chunk->cursor].type = type;
    chunk->list[chunk->cursor].loc = (Loc) loc;
    chunk->cursor++;
}

void push_token_str(TokenList* chunk, int type, char* str, int size, Loc loc)
{
    if (chunk->cursor == chunk->capacity){
        chunk->capacity += 16;
        chunk->list = realloc(chunk->list, chunk->capacity * sizeof(Token));
    }

    chunk->list[chunk->cursor].type = type;
    chunk->list[chunk->cursor].str = calloc(size+1, sizeof(char));

    for (int i = 0; i < size; i++)
        chunk->list[chunk->cursor].str[i] = str[i];
    
    chunk->list[chunk->cursor].str[size] = '\0';
    chunk->list[chunk->cursor].loc = loc;
    chunk->cursor++;
}

void push_token_num(TokenList* chunk, int type, mpz_t num)
{
    if (chunk->cursor == chunk->capacity){
        chunk->capacity += 16;
        chunk->list = realloc(chunk->list, chunk->capacity * sizeof(Token));
    }

    chunk->list[chunk->cursor].type = type;
    //mpz_set(chunk->list[chunk->cursor].num, num);
    chunk->list[chunk->cursor].loc = (Loc) {0, 0, 0, 0};
    chunk->cursor++;
}

void print_token(Token t)
{
    switch (t.type) {
        case TOKEN_ID : printf("TOKEN_ID %s", t.str); break;
        case TOKEN_STRING : printf("TOKEN_STRING %s", t.str); break;
        case TOKEN_INT : printf("TOKEN_INT"); /*gmp_printf ("  %Zd", t.num)*/; break;
        case TOKEN_SLASH : printf("TOKEN_SLASH"); break;
        case TOKEN_STAR : printf("TOKEN_STAR"); break;
        case TOKEN_PLUS : printf("TOKEN_PLUS"); break;
        case TOKEN_MINUS : printf("TOKEN_MINUS"); break;
        case TOKEN_DIV : printf("TOKEN_DIV"); break;
        case TOKEN_EQ : printf("TOKEN_EQ"); break;
        case TOKEN_LT : printf("TOKEN_LT"); break;
        case TOKEN_GT : printf("TOKEN_GT"); break;
        case TOKEN_COMMA : printf("TOKEN_COMMA"); break;
        case TOKEN_COLON : printf("TOKEN_COLON"); break;
        case TOKEN_LPAREN : printf("TOKEN_LPAREN"); break;
        case TOKEN_RPAREN : printf("TOKEN_RPAREN"); break;
        case TOKEN_LBRACKET : printf("TOKEN_LBRACKET"); break;
        case TOKEN_RBRACKET : printf("TOKEN_RBRACKET"); break;
        case TOKEN_LSQBRACKET : printf("TOKEN_LSQBRACKET"); break;
        case TOKEN_RSQBRACKET : printf("TOKEN_RSQBRACKET"); break;
        case TOKEN_ASSIGN : printf("TOKEN_ASSIGN"); break;
        case TOKEN_PROGRAM : printf("TOKEN_PROGRAM"); break;  
        case TOKEN_FUNCTION : printf("TOKEN_FUNCTION"); break;
        case TOKEN_PROC : printf("TOKEN_PROC"); break;
        case TOKEN_VAR : printf("TOKEN_VAR"); break;
        case TOKEN_BEGIN : printf("TOKEN_BEGIN"); break;
        case TOKEN_END : printf("TOKEN_END"); break;
        case TOKEN_IF : printf("TOKEN_IF"); break;
        case TOKEN_ELSE : printf("TOKEN_ELSE"); break;
        case TOKEN_THEN : printf("TOKEN_THEN"); break;
        case TOKEN_REPEAT : printf("TOKEN_REPEAT"); break;
        case TOKEN_WHILE : printf("TOKEN_WHILE"); break;
        case TOKEN_THAN : printf("TOKEN_THAN"); break;
        case TOKEN_EOF : printf("TOKEN_EOF"); break;
        default: printf("UNKOWN_TOKEN"); break; // error triger
    }

    printf("\t\t\t%ld,%ld-%ld,%ld\n",t.loc.sl, t.loc.sc, t.loc.el, t.loc.ec);
}


void print_token_list(TokenList* chunk)
{
    for (int i = 0; i < chunk->cursor; i++)
        print_token(chunk->list[i]);
}

void pop_token_list(TokenList* chunk) 
{
    free(chunk->list);
    chunk->list = NULL;
    free(chunk);
}