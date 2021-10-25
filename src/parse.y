%{
#include <gmp.h>
#include <stdio.h>
#include <stdlib.h>

#include "ast.h"
#include "vec.h"

#include "parse.h"
#include "scan.h"

ASTProgram *ast;

static ASTExpression *mk_bin_expr(ASTBinaryOperator op, ASTExpression *left, ASTExpression *right);

void yyerror(const char *s);
%}

%union {
    ASTProgram *prog;
    ASTSubroutine *sub;
    ASTVariableDecl decl;
    ASTStatement *stmt;
    ASTExpression *expr;
    ASTLvalue lval;
    mpz_t integer;
    char *str;
    char *ident;
    Vec vec;
    int token;
}

%token TOKEN_PROGRAM TOKEN_FUNCTION TOKEN_PROCEDURE TOKEN_VARIABLES TOKEN_BEGIN TOKEN_END
%token TOKEN_ASSIGNMENT TOKEN_LPAREN TOKEN_RPAREN TOKEN_COMMA TOKEN_COLON
%token TOKEN_PLUS TOKEN_MINUS TOKEN_STAR TOKEN_DIV TOKEN_MOD
%token <integer> TOKEN_INTEGER_LITTERAL
%token <str> TOKEN_STRING_LITTERAL
%token <ident> TOKEN_IDENTIFIER

%type <prog> program
%type <vec> subroutine_sequence subroutine_parameter_sequence statement_sequence variable_decl_sequence variable_decl_block arguments argument_sequence
%type <sub> subroutine
%type <decl> variable_decl
%type <stmt> statement
%type <expr> expression
%type <lval> lvalue
%type <integer> integer_litteral
%type <str> string_litteral
%type <ident> identifier

%nonassoc TOKEN_LPAREN TOKEN_RPAREN
%left TOKEN_PLUS TOKEN_MINUS
%left TOKEN_STAR TOKEN_DIV TOKEN_MOD

%start program

%%

program
    : TOKEN_PROGRAM identifier
      subroutine_sequence
      variable_decl_block
      TOKEN_BEGIN
      statement_sequence
      TOKEN_END
      {
          ast = calloc(1, sizeof *ast);
          ast->name = $2;
          ast->subroutines = $3;
          ast->variables = $4;
          ast->statements = $6;
      }
    ;

subroutine_sequence
    : %empty { $$ = VEC_EMPTY; }
    | subroutine_sequence subroutine { $$ = $1; vec_push(&$$, $2); }
    ;

subroutine
    : TOKEN_FUNCTION identifier
      TOKEN_LPAREN
      subroutine_parameter_sequence
      TOKEN_RPAREN
      TOKEN_COLON
      identifier
      variable_decl_block
      TOKEN_BEGIN
      statement_sequence
      TOKEN_END
      {
          $$ = malloc(sizeof *$$);
          $$->name = $2;
          $$->kind = AST_SUB_FUNCTION;
          $$->parameters = $4;
          $$->return_type.name = $7;
          $$->variables = $8;
          $$->statements = $10;
      }
    ;

statement_sequence
    : %empty { $$ = VEC_EMPTY; }
    | statement_sequence statement { $$ = $1; vec_push(&$$, $2); }
    ;

statement
    : lvalue TOKEN_ASSIGNMENT expression
      {
          $$ = malloc(sizeof *$$);
          $$->kind = AST_STMT_ASSIGNMENT;
          $$->lvalue = $1;
          $$->rvalue = $3;
      }
    | expression
      {
          $$ = malloc(sizeof *$$);
          $$->kind = AST_STMT_EXPRESSION;
          $$->expression = $1;
      }
    ;

subroutine_parameter_sequence
    : %empty { $$ = VEC_EMPTY; }
    | subroutine_parameter_sequence TOKEN_COMMA variable_decl
      {
          ASTSubroutineParameter *param = malloc(sizeof *param);
          param->kind = AST_PARAM_IN;
          param->decl = $3;
          $$ = $1;
          vec_push(&$$, param);
      }
    ;

variable_decl_block
    : %empty { $$ = VEC_EMPTY; }
    | TOKEN_VARIABLES variable_decl_sequence { $$ = $2; }
    ;

variable_decl_sequence
    : %empty { $$ = VEC_EMPTY; }
    | variable_decl_sequence variable_decl
      {
          ASTVariableDecl *decl = malloc(sizeof *decl);
          *decl = $2;
          $$ = $1;
          vec_push(&$$, decl);
      }
    ;

variable_decl
    : identifier TOKEN_COLON identifier
      {
          $$.variable.name = $1;
          $$.type.name = $3;
      }
    ;

lvalue
    : identifier
      {
          $$.kind = AST_LVALUE_VARIABLE;
          $$.variable.name = $1;
      }
    ;

expression
    : TOKEN_LPAREN expression TOKEN_RPAREN { $$ = $2; }
    | integer_litteral
      {
          $$ = malloc(sizeof *$$);
          $$->kind = AST_EXPR_INTEGER_LITTERAL;
          mpz_init_set($$->integer_litteral, $1);
          mpz_clear($1);
      }
    | string_litteral
      {
          $$ = malloc(sizeof *$$);
          $$->kind = AST_EXPR_STRING_LITTERAL;
          $$->string_litteral = $1;
      }
    | identifier
      {
          $$ = malloc(sizeof *$$);
          $$->kind = AST_EXPR_VARIABLE;
          $$->variable.name = $1;
      }
    | expression TOKEN_PLUS expression { $$ = mk_bin_expr(AST_OP_ADD, $1, $3); }
    | expression TOKEN_MINUS expression { $$ = mk_bin_expr(AST_OP_SUB, $1, $3); }
    | expression TOKEN_STAR expression { $$ = mk_bin_expr(AST_OP_MUL, $1, $3); }
    | expression TOKEN_DIV expression { $$ = mk_bin_expr(AST_OP_DIV, $1, $3); }
    | expression TOKEN_MOD expression { $$ = mk_bin_expr(AST_OP_MOD, $1, $3); }
    | identifier TOKEN_LPAREN arguments TOKEN_RPAREN
      {
          $$ = malloc(sizeof *$$);
          $$->kind = AST_EXPR_SUBROUTINE_CALL;
          $$->subroutine_name = $1;
          $$->arguments = $3;
      }
    ;

arguments
    : %empty { $$ = VEC_EMPTY; }
    | argument_sequence
    ;

argument_sequence
    : expression { $$ = VEC_EMPTY; vec_push(&$$, $1); }
    | argument_sequence TOKEN_COMMA expression { $$ = $1; vec_push(&$$, $3); }
    ;

integer_litteral
    : TOKEN_INTEGER_LITTERAL
    ;

string_litteral
    : TOKEN_STRING_LITTERAL
    ;

identifier
    : TOKEN_IDENTIFIER
    ;

%%

static ASTExpression *mk_bin_expr(ASTBinaryOperator op, ASTExpression *left, ASTExpression *right) {
    ASTExpression *expr = malloc(sizeof *expr);
    expr->kind = AST_EXPR_BINARY_EXPRESSION;
    expr->binary_expression.left = left;
    expr->binary_expression.op = op;
    expr->binary_expression.right = right;
    return expr;
}
