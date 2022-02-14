/**
 * @file lexer.c
 * @date 10/02/2022
 * @brief
 * @todo check full line 1024 char
*/
#include "lexer.h"

int utfsize(char* c)
{
    int i = 7;
    int size = 0;
    
    while (((c[0] >> i) & 1) && i >= 0) {
        size++;
        i--;
    }

    return size;
}

int utfcomp(char* c, char* ref, int ref_size)
{
    int size = utfsize(c);
    
    if (ref_size != size)
        return 0;

    for (int i = 0; i < size; i++) {
        if (c[i] != ref[i])
            return 0;
    }
    
    return 1;
}

void lexer_normalize_content(char* str)
{
    int i = 0, j = 0;
    char* ptr = str;
    char line[1024];
    int offset = 0;
    const int size = strlen(str);

    while (i < size) {
        offset = utfsize(str);
        if (str[0] == '\"') {
            do {
                line[j] = str[0];
                j++;
                i++;
                str++;
            } while (i < size && str[0] != '\"');
            line[j] = '\"';
            j++;
            i++;
            str++;
        } else if (offset != 0) {
            if (utfcomp(str,"à",2)) line[j] = 'a';
            else if (utfcomp(str,"á",2)) line[j] = 'a';
            else if (utfcomp(str,"á",2)) line[j] = 'a';
            else if (utfcomp(str,"â",2)) line[j] = 'a';
            else if (utfcomp(str,"ä",2)) line[j] = 'a';
            //case æ
            else if (utfcomp(str,"ç",2)) line[j] = 'c';
            else if (utfcomp(str,"è",2)) line[j] = 'e';
            else if (utfcomp(str,"é",2)) line[j] = 'e';
            else if (utfcomp(str,"ê",2)) line[j] = 'e';
            else if (utfcomp(str,"ë",2)) line[j] = 'e';
            else if (utfcomp(str,"î",2)) line[j] = 'i';
            else if (utfcomp(str,"ï",2)) line[j] = 'i';
            else if (utfcomp(str,"ô",2)) line[j] = 'o';
            else if (utfcomp(str,"ö",2)) line[j] = 'o';
            else if (utfcomp(str,"û",2)) line[j] = 'u';
            else if (utfcomp(str,"ü",2)) line[j] = 'u';
            else if (utfcomp(str,"ÿ",2)) line[j] = 'y';

            else if (utfcomp(str,"À",2)) line[j] = 'a';
            else if (utfcomp(str,"Á",2)) line[j] = 'a';
            else if (utfcomp(str,"Â",2)) line[j] = 'a';
            else if (utfcomp(str,"Ä",2)) line[j] = 'a';
            // case Æ
            else if (utfcomp(str,"Ç",2)) line[j] = 'c';
            else if (utfcomp(str,"È",2)) line[j] = 'e';
            else if (utfcomp(str,"É",2)) line[j] = 'e';
            else if (utfcomp(str,"Ê",2)) line[j] = 'e';
            else if (utfcomp(str,"Ë",2)) line[j] = 'e';
            else if (utfcomp(str,"Î",2)) line[j] = 'i';
            else if (utfcomp(str,"Ï",2)) line[j] = 'i';
            else if (utfcomp(str,"Ô",2)) line[j] = 'o';
            else if (utfcomp(str,"Ö",2)) line[j] = 'o';
            else if (utfcomp(str,"×",2)) line[j] = '*';
            else if (utfcomp(str,"Û",2)) line[j] = 'u';
            else if (utfcomp(str,"Ü",2)) line[j] = 'u';
            //case ←
            else {
                //error triger;
            }

            j++;
            i+=offset;
            str+=offset;
        } else {
            if (str[0] < 91 && str[0] > 64)
                line[j] = str[0] + ' ';
            else 
                line[j] = str[0];
            j++;
            i++;
            str++;
        }
    }

    line[j] = '\0';
    for (i = 0; i < (j+1); i++)
        ptr[i] = line[i];
}

void get_line(Lexer* lexer)
{
    if (fgets(lexer->line, LINE_MAX, lexer->fs) == NULL) {
        if (feof(lexer->fs))
            lexer->c = EOF;
    } else {
  

        lexer_normalize_content(lexer->line);

        printf("%s", lexer->line);

        lexer->line_nb++;
        lexer->line_cursor = 0;

        lexer->c = lexer->line[0];
    }
}

Lexer* init_lexer_from_file(FILE *fs)
{
    Lexer* res = calloc(1, sizeof(Lexer));
    res->fs = fs;
    res->line_nb = 0;

    get_line(res);
    
    return res;
}

void lexer_advance(Lexer* lexer)
{
    if (lexer->c == '\n') {
        get_line(lexer);
    } else if (lexer->c != EOF) {
        lexer->line_cursor++;
        lexer->c = lexer->line[lexer->line_cursor];
    }
}

void lexer_skip_whitespace(Lexer* lexer)
{
    while (lexer->c == ' ' || lexer->c == '\t')
        lexer_advance(lexer);
}

int lexer_check_next(Lexer* lexer, char c)
{
    return ((lexer->line_cursor + 1) < strlen(lexer->line) && lexer->line[lexer->line_cursor+1] == c);
}

void lexer_advance_with(Lexer* lexer, TokenList* chunk, int type)
{
    push_token(chunk, type);
    lexer_advance(lexer);
}

int lexer_compare(Lexer* lexer, int start, char* comp)
{
    if (strlen(comp) != (lexer->line_cursor-start))
        return 0;

    for (int i = start; i < lexer->line_cursor; i++){
        if (lexer->line[i] != comp[i-start])
            return 0;
    }

    return 1;
}

/*
void lexer_scan_sting(Lexer* lexer, TokenList* chunk)
{
    int i = 0;
    char buffer[10]

    wh
}
*/

void lexer_scan_id(Lexer* lexer, TokenList* chunk)
{
    int start = lexer->line_cursor;
    printf("%c\n",lexer->c);

    while (lexer->c == '_' || (lexer->c > 96 && lexer->c < 123))
        lexer_advance(lexer);
    
    if (lexer->c != EOF) {
        if (lexer_compare(lexer, start, "programme")) push_token(chunk, TOKEN_PROGRAM);
        else if (lexer_compare(lexer, start, "fonction")) push_token(chunk, TOKEN_FUNCTION);
        else if (lexer_compare(lexer, start, "procedure")) push_token(chunk, TOKEN_PROC);
        else if (lexer_compare(lexer, start, "variable")) push_token(chunk, TOKEN_VAR);
        else if (lexer_compare(lexer, start, "debut")) push_token(chunk, TOKEN_BEGIN);
        else if (lexer_compare(lexer, start, "fin")) push_token(chunk, TOKEN_END);
        else push_token(chunk, TOKEN_ID);
    }
}

TokenList* lexer_scan(void)
{
    TokenList* res = init_token_list();

    FILE* fs = fopen("./tests/if.bl", "r");
    Lexer* lexer = init_lexer_from_file(fs);

    while (lexer->c != EOF) {
        lexer_skip_whitespace(lexer);

        if (lexer->c == '_' || (lexer->c > 96 && lexer->c < 123)) {
            lexer_scan_id(lexer, res);
        } else {
            switch (lexer->c) {
                //case '\"':
                //case '/':
                case '*' : lexer_advance_with(lexer, res, TOKEN_STAR); break;
                case '+' : lexer_advance_with(lexer, res, TOKEN_PLUS); break;
                case '-' : lexer_advance_with(lexer, res, TOKEN_MINUS); break;
                case '=' : lexer_advance_with(lexer, res, TOKEN_EQ); break;
                case '<' : lexer_advance_with(lexer, res, lexer_check_next(lexer,'-') ? TOKEN_ASSIGN : TOKEN_LT); break;
                case '>' : lexer_advance_with(lexer, res, TOKEN_GT); break;
                case ',' : lexer_advance_with(lexer, res, TOKEN_COMMA); break;
                case ':' : lexer_advance_with(lexer, res, TOKEN_COLON); break;
                case '(' : lexer_advance_with(lexer, res, TOKEN_LPAREN); break;
                case ')' : lexer_advance_with(lexer, res, TOKEN_RPAREN); break;
                case '{' : lexer_advance_with(lexer, res, TOKEN_LBRACKET); break;
                case '}' : lexer_advance_with(lexer, res, TOKEN_RBRACKET); break;
                case '[' : lexer_advance_with(lexer, res, TOKEN_LSQBRACKET); break;
                case ']' : lexer_advance_with(lexer, res, TOKEN_RSQBRACKET); break;
                default : lexer_advance(lexer); //error trigger
            }
        }
    }    
    print_token_list(res);

    return res;
}