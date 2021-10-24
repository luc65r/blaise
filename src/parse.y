%{
#include <stdio.h>
#include <stdlib.h>

#include "ast.h"
#include "vec.h"

#include "parse.h"
#include "scan.h"

ASTProgram *ast;

void yyerror(const char *s) {
    fprintf(stderr, "error: %s\n", s);
}
%}

%union {
    ASTProgram *prog;
    ASTSubroutine *sub;
    ASTVariableDecl decl;
    ASTStatement *stmt;
    ASTExpression *expr;
    ASTLvalue lval;
    ASTBinaryOperator op;
    char *ident;
    Vec vec;
    int token;
}

%token <token> TOKEN_PROGRAM TOKEN_FUNCTION TOKEN_PROCEDURE TOKEN_VARIABLES TOKEN_BEGIN TOKEN_END
%token <token> TOKEN_ASSIGNMENT TOKEN_LPAREN TOKEN_RPAREN TOKEN_COMMA TOKEN_COLON
%token <token> TOKEN_PLUS
%token <ident> TOKEN_IDENTIFIER

%type <prog> program
%type <vec> subroutine_sequence subroutine_parameter_sequence statement_sequence variable_decl_sequence variable_decl_block argument_sequence
%type <sub> subroutine
%type <decl> variable_decl
%type <stmt> statement
%type <expr> expression
%type <lval> lvalue
%type <op> binary_operator
%type <ident> identifier

%left TOKEN_PLUS

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
    : identifier
      {
          $$ = malloc(sizeof *$$);
          $$->kind = AST_EXPR_VARIABLE;
          $$->variable.name = $1; 
      }
    | expression binary_operator expression
      {
          $$ = malloc(sizeof *$$);
          $$->kind = AST_EXPR_BINARY_EXPRESSION;
          $$->binary_expression.left = $1;
          $$->binary_expression.op = $2;
          $$->binary_expression.right = $3;
      }
    | identifier TOKEN_LPAREN argument_sequence TOKEN_RPAREN
      {
          $$ = malloc(sizeof *$$);
          $$->kind = AST_EXPR_SUBROUTINE_CALL;
          $$->subroutine_name = $1;
          $$->arguments = $3;
      }
    ;

argument_sequence
    : %empty { $$ = VEC_EMPTY; }
    | argument_sequence TOKEN_COMMA expression { $$ = $1; vec_push(&$$, $3); }
    ;

binary_operator
    : TOKEN_PLUS { $$ = AST_OP_ADD; }
    ;

identifier
    : TOKEN_IDENTIFIER
    ;

%%
