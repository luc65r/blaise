%{
#include <assert.h>
#include <gmp.h>
#include <stdio.h>
#include <stdlib.h>

#include "ast.h"
#include "vec.h"

#include "parse.h"
#include "scan.h"

#define YYLLOC_DEFAULT(C, R, N) do {                \
        if (N) {                                    \
            (C).sl = YYRHSLOC(R, 1).sl;             \
            (C).sc = YYRHSLOC(R, 1).sc;             \
            (C).el = YYRHSLOC(R, N).el;             \
            (C).ec = YYRHSLOC(R, N).ec;             \
        } else {                                    \
            (C).sl = (C).el = YYRHSLOC(R, 0).el;    \
            (C).sc = (C).ec = YYRHSLOC(R, 0).ec;    \
        }                                           \
    } while (0)

#define ALLOC(T, S...) ({                       \
            T _t = { S };            \
            T *_pt = malloc(sizeof (T));        \
            assert(_pt != NULL);                \
            *_pt = _t;                          \
            _pt;                                \
        })

#define SVEC(T, N, V)                           \
    .n ## N = (V).len,                          \
        .N = (T **)(V).elems

#define BINEXPR(O, L, A, B) \
    ALLOC(ASTBinExpr, L, .op = O, .left = A, .right = B)

ASTProg *ast;

void yyerror(const char *s);
%}

%define api.token.prefix {TOKEN_}
%define api.location.type {ASTLoc}

%define parse.trace
%define parse.error detailed
%locations

%initial-action {
    @$ = (YYLTYPE){ 1, 1, 1, 1 };
};

%union {
    ASTProg *prog;
    ASTSub *sub;
    ASTSubParam *subparam;
    ASTVarDecl *decl;
    ASTStmt *stmt;
    ASTForBlock *forb;
    ASTDoWhileBlock *dowhileb;
    ASTWhileBlock *whileb;
    ASTIfElseBlock *ifelseb;
    ASTLval *lval;
    ASTExpr *expr;
    ASTBinExpr *binexpr;
    ASTType *type;
    ASTVar *var;
    mpz_t integer;
    bool boolean;
    char *str;
    char *ident;
    Vec vec;
    int token;
}

%token PROGRAM FUNCTION PROCEDURE VARIABLES BEGIN END
%token IF THEN ELSE FOR FROM TO DO WHILE
%token QUE REPEAT RETURN TRUE FALSE
%token EQ NEQ LE LT GE GT
%token ASSIGNMENT LPAREN RPAREN COMMA COLON
%token PLUS MINUS STAR DIV MOD
%token <integer> INTLIT
%token <str> STRLIT
%token <ident> IDENT

%type <prog> program
%type <sub> subroutine
%type <subparam> subroutine_parameter
%type <decl> variable_decl
%type <stmt> statement
%type <forb> for_block
%type <dowhileb> do_while_block
%type <whileb> while_block
%type <ifelseb> if_block else_block
%type <lval> lvalue
%type <expr> expression
%type <binexpr> binary_expression
%type <type> type
%type <var> variable
%type <integer> integer_litteral
%type <boolean> boolean_litteral
%type <str> string_litteral
%type <ident> identifier
%type <vec> subroutine_sequence subroutine_parameter_sequence statement_sequence variable_decl_sequence variable_decl_block arguments argument_sequence

%nonassoc LPAREN RPAREN
%left OR
%left AND
%left EQ NEQ
%left GT GE LT LE
%left PLUS MINUS
%left STAR DIV MOD

%start program

%%

program
    : PROGRAM identifier
      subroutine_sequence
      variable_decl_block
      BEGIN
      statement_sequence
      END
    {
        ast = ALLOC(ASTProg, @$,
                    .name = $2,
                    SVEC(ASTSub, subs, $3),
                    SVEC(ASTVarDecl, vars, $4),
                    SVEC(ASTStmt, stmts, $6));
    }
    ;

subroutine_sequence
    : %empty { $$ = VEC_EMPTY; }
    | subroutine_sequence subroutine { $$ = $1; vec_push(&$$, $2); }
    ;

subroutine
    : FUNCTION identifier
      LPAREN
      subroutine_parameter_sequence
      RPAREN
      COLON
      type
      variable_decl_block
      BEGIN
      statement_sequence
      END
      {
          $$ = ALLOC(ASTSub, @$,
                     .name = $2,
                     .kind = AST_SUB_FUNC,
                     SVEC(ASTSubParam, params, $4),
                     .rtype = $7,
                     SVEC(ASTVarDecl, vars, $8),
                     SVEC(ASTStmt, stmts, $10));
      }
    ;

statement_sequence
    : %empty { $$ = VEC_EMPTY; }
    | statement_sequence statement { $$ = $1; vec_push(&$$, $2); }
    ;

statement
    : lvalue ASSIGNMENT expression
      {
        $$ = ALLOC(ASTStmt, @$, .kind = AST_STMT_ASSIGNMENT, .lval = $1, .rval = $3);
      }
    | expression { $$ = ALLOC(ASTStmt, @$, .kind = AST_STMT_EXPR, .expr = $1); }
    | RETURN expression { $$ = ALLOC(ASTStmt, @$, .kind = AST_STMT_RETURN, .expr = $2); }
    | if_block { $$ = ALLOC(ASTStmt, @$, .kind = AST_STMT_IF, .ifb = $1); }
    | while_block { $$ = ALLOC(ASTStmt, @$, .kind = AST_STMT_WHILE, .whileb = $1); }
    | do_while_block { $$ = ALLOC(ASTStmt, @$, .kind = AST_STMT_DO_WHILE, .dowhileb = $1); }
    | for_block { $$ = ALLOC(ASTStmt, @$, .kind = AST_STMT_FOR, .forb = $1); }
    ;

if_block
    : IF expression THEN statement_sequence else_block
      {
          $$ = ALLOC(ASTIfElseBlock, @$, .cond = $2, SVEC(ASTStmt, stmts, $4), .elseb = $5);
      }
    ;

else_block
    : END IF { $$ = NULL; }
    | ELSE if_block { $$ = $2; }
    | ELSE statement_sequence END IF
      {
          $$ = ALLOC(ASTIfElseBlock, @$, .cond = NULL, SVEC(ASTStmt, stmts, $2), .elseb = NULL);
      }
    ;

while_block
    : WHILE QUE expression DO statement_sequence END WHILE QUE
      {
          $$ = ALLOC(ASTWhileBlock, @$, .cond = $3, SVEC(ASTStmt, stmts, $5));
      }
    ;

do_while_block
    : REPEAT statement_sequence WHILE QUE expression
      {
          $$ = ALLOC(ASTDoWhileBlock, @$, .cond = $5, SVEC(ASTStmt, stmts, $2));
      }
    ;

for_block
    : FOR lvalue FROM expression TO expression DO statement_sequence END FOR
      {
          $$ = ALLOC(ASTForBlock, @$, .iter = $2, .from = $4, .to = $6, SVEC(ASTStmt, stmts, $8));
      }
    ;

subroutine_parameter_sequence
    : %empty { $$ = VEC_EMPTY; }
    | subroutine_parameter_sequence COMMA subroutine_parameter { $$ = $1; vec_push(&$$, $3); }
    ;

subroutine_parameter
    : variable_decl { $$ = ALLOC(ASTSubParam, @$, .kind = AST_PARAM_IN, .decl = $1); }
    ;

variable_decl_block
    : %empty { $$ = VEC_EMPTY; }
    | VARIABLES variable_decl_sequence { $$ = $2; }
    ;

variable_decl_sequence
    : %empty { $$ = VEC_EMPTY; }
    | variable_decl_sequence variable_decl { $$ = $1; vec_push(&$$, $2); }
    ;

variable_decl
    : variable COLON type { $$ = ALLOC(ASTVarDecl, @$, .var = $1, .type = $3); }
    ;

lvalue
    : variable { $$ = ALLOC(ASTLval, @$, .kind = AST_LVAL_VAR, .var = $1); }
    ;

expression
    : LPAREN expression RPAREN { $$ = $2; }
    | integer_litteral
      {
          $$ = ALLOC(ASTExpr, @$, .kind = AST_EXPR_INTLIT);
          mpz_init_set($$->intlit, $1);
          mpz_clear($1);
      }
    | boolean_litteral { $$ = ALLOC(ASTExpr, @$, .kind = AST_EXPR_BOOLLIT, .boollit = $1); }
    | string_litteral { $$ = ALLOC(ASTExpr, @$, .kind = AST_EXPR_STRLIT, .strlit = $1); }
    | variable { $$ = ALLOC(ASTExpr, @$, .kind = AST_EXPR_VAR, .var = $1); }
    | identifier LPAREN arguments RPAREN
      {
          $$ = ALLOC(ASTExpr, @$, .kind = AST_EXPR_CALL,
                     .subname = $1, SVEC(ASTExpr, args, $3));
      }
    | binary_expression { $$ = ALLOC(ASTExpr, @$, .kind = AST_EXPR_BINEXPR, .binexpr = $1); }
    ;

binary_expression
    : expression OR expression { $$ = BINEXPR(AST_OP_OR, @$, $1, $3); }
    | expression AND expression { $$ = BINEXPR(AST_OP_AND, @$, $1, $3); }
    | expression EQ expression { $$ = BINEXPR(AST_OP_EQ, @$, $1, $3); }
    | expression NEQ expression { $$ = BINEXPR(AST_OP_NEQ, @$, $1, $3); }
    | expression GT expression { $$ = BINEXPR(AST_OP_GT, @$, $1, $3); }
    | expression GE expression { $$ = BINEXPR(AST_OP_GE, @$, $1, $3); }
    | expression LT expression { $$ = BINEXPR(AST_OP_LT, @$, $1, $3); }
    | expression LE expression { $$ = BINEXPR(AST_OP_LE, @$, $1, $3); }
    | expression PLUS expression { $$ = BINEXPR(AST_OP_ADD, @$, $1, $3); }
    | expression MINUS expression { $$ = BINEXPR(AST_OP_SUB, @$, $1, $3); }
    | expression STAR expression { $$ = BINEXPR(AST_OP_MUL, @$, $1, $3); }
    | expression DIV expression { $$ = BINEXPR(AST_OP_DIV, @$, $1, $3); }
    | expression MOD expression { $$ = BINEXPR(AST_OP_MOD, @$, $1, $3); }
    ;

arguments
    : %empty { $$ = VEC_EMPTY; }
    | argument_sequence
    ;

argument_sequence
    : expression { $$ = VEC_EMPTY; vec_push(&$$, $1); }
    | argument_sequence COMMA expression { $$ = $1; vec_push(&$$, $3); }
    ;

variable
    : identifier { $$ = ALLOC(ASTVar, @$, .name = $1); }
    ;

type
    : identifier { $$ = ALLOC(ASTType, @$, .name = $1); }
    ;

integer_litteral
    : INTLIT
    ;

boolean_litteral
    : TRUE { $$ = true; }
    | FALSE { $$ = false; }
    ;

string_litteral
    : STRLIT
    ;

identifier
    : IDENT
    ;

%%
