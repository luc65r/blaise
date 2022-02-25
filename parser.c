/**
 * @file parser.c
 * @date
 * @brief
*/
#include "parser.h"

char* cpystr(char* str)
{   
    const int length = strlen(str) + 1;
    char* res = malloc(length * sizeof(char));

    for (int i = 0; i < length; i++)
        res[i] = str[i];
    
    return res;
}

Parser* init_parser(TokenList* tlist)
{
    Parser* res = malloc(sizeof(Parser));
    res->list = tlist->list;
    res->cursor = 0;
    res->t = res->list[res->cursor];

    return res;
}

void parser_advance(Parser* parser)
{
    if (parser->t.type != TOKEN_EOF) {
        parser->cursor++;
        parser->t = parser->list[parser->cursor];
    }
}

int parser_check_end(Parser* parser, int pos) 
{
    for (int i = 0; i < pos; i++) {
        if (parser->list[parser->cursor+i].type == TOKEN_EOF)
            return 0;
    }
    
    return 1;
}

int parser_check_next(Parser* parser, int pos, int type) {
    if(!parser_check_end(parser, pos))
        return 0;

    return (parser->list[parser->cursor+pos].type == type);
}

void parse_prog_name(ASTProg* prog, Parser* parser)
{
    if (parser->t.type == TOKEN_PROGRAM) {
        parser_advance(parser);
        if(parser->t.type == TOKEN_ID) {
            prog->name = cpystr(parser->t.str);
            parser_advance(parser);
        }
    }
}

void parse_var(ASTVarDecl** vars, size_t* num, Parser* parser)
{
    int cursor = 0;

    while (parser->t.type == TOKEN_ID && parser_check_next(parser, 1, TOKEN_COLON) && parser_check_next(parser, 2, TOKEN_ID)) {
        if (cursor == *num) {
            (*num)++;
            vars = realloc(vars, (*num)*sizeof(ASTVarDecl*));
            vars[cursor] = malloc(sizeof(ASTVarDecl));
        }

        vars[cursor]->var = malloc(sizeof(ASTVar));
        vars[cursor]->var->name = cpystr(parser->t.str);
        vars[cursor]->var->loc = parser->t.loc;
        parser_advance(parser);
        parser_advance(parser);
        
        vars[cursor]->type = malloc(sizeof(ASTType));
        vars[cursor]->type->name = cpystr(parser->t.str);
        vars[cursor]->type->loc = parser->t.loc;
        parser_advance(parser);

        vars[cursor]->loc = (Loc) {vars[cursor]->var->loc.sl, vars[cursor]->var->loc.sc, vars[cursor]->type->loc.el, vars[cursor]->type->loc.ec};
        cursor++;
    }
    
    if (cursor == 0) {
        free(*vars);
        free(vars);
        *num = 0;
    }
}

void parse_stmt(ASTStmt** stmts, size_t* num, Parser* parser)
{
    int cursor = 0;

    while (parser->t.type != TOKEN_END) {
        if (cursor == *num) {
            (*num)++;
            stmts = realloc(stmts, (*num)*sizeof(ASTStmt*));
            stmts[cursor] = malloc(sizeof(ASTStmt));
        }

        cursor++;
    }
    
    if (cursor == 0) {
        free(*stmts);
        free(stmts);
        *num = 0;
    }
}

void parse_program(ASTProg* prog, Parser* parser)
{

    if (parser->t.type == TOKEN_VAR) {
        parser_advance(parser);
        prog->vars = malloc(sizeof(ASTVarDecl*));
        *(prog->vars) = malloc(sizeof(ASTVarDecl));
        prog->nvars = 1;
        parse_var(prog->vars, &(prog->nvars), parser);
    }

}

ASTProg* parse(TokenList* list)
{
    Parser* parser = init_parser(list);
    ASTProg* res = malloc(sizeof(ASTProg));
    res->loc.sl = parser->t.loc.sl;
    res->loc.sc = parser->t.loc.sc;

    parse_prog_name(res, parser);
    parse_program(res, parser);

    json_t *j = ast_json(res);
    json_dumpf(j, stdout, JSON_INDENT(2));
    printf("\n");
    json_decref(j);

    return res;
}