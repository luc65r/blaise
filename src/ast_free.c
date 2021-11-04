#include <assert.h>
#include <gmp.h>
#include <stdlib.h>

#include "ast.h"

#define FDECL(T, A) static void ast_ ## A ## _free(T *a)

#define FSTART(T, A) static void ast_ ## A ## _free(T *a) {  \
    assert(a != NULL);
#define FEND free(a);                           \
    }

#define FARRAY(X, Y, F) do {                            \
        for (size_t _i = 0; _i < (X)->n ## Y; _i++) {   \
            (F)((X)->Y[_i]);                            \
        }                                               \
        free((X)->Y);                                   \
    } while (0)

FDECL(ASTVar, var);
FDECL(ASTType, type);
FDECL(ASTBinExpr, binexpr);
FDECL(ASTExpr, expr);
FDECL(ASTLval, lval);
FDECL(ASTIfElseBlock, if);
FDECL(ASTWhileBlock, while);
FDECL(ASTDoWhileBlock, dowhile);
FDECL(ASTForBlock, for);
FDECL(ASTStmt, stmt);
FDECL(ASTVarDecl, decl);
FDECL(ASTSubParam, param);
FDECL(ASTSub, sub);

FSTART(ASTVar, var)
     free(a->name);
FEND

FSTART(ASTType, type)
     free(a->name);
FEND

FSTART(ASTBinExpr, binexpr)
     ast_expr_free(a->left);
     ast_expr_free(a->right);
FEND

FSTART(ASTExpr, expr)
     switch (a->kind) {
     case AST_EXPR_INTLIT:
         mpz_clear(a->intlit);
         break;
     case AST_EXPR_STRLIT:
         free(a->strlit);
         break;
     case AST_EXPR_BOOLLIT:
         break;
     case AST_EXPR_VAR:
         ast_var_free(a->var);
         break;
     case AST_EXPR_BINEXPR:
         ast_binexpr_free(a->binexpr);
         break;
     case AST_EXPR_CALL:
         free(a->subname);
         FARRAY(a, args, ast_expr_free);
         break;
     }
FEND

FSTART(ASTLval, lval)
     switch (a->kind) {
     case AST_LVAL_VAR:
         ast_var_free(a->var);
         break;
     }
FEND

FSTART(ASTIfElseBlock, if)
     if (a->cond != NULL)
         ast_expr_free(a->cond);
     FARRAY(a, stmts, ast_stmt_free);
     if (a->elseb != NULL)
         ast_if_free(a->elseb);
FEND

FSTART(ASTWhileBlock, while)
     ast_expr_free(a->cond);
     FARRAY(a, stmts, ast_stmt_free);
FEND

FSTART(ASTDoWhileBlock, dowhile)
     FARRAY(a, stmts, ast_stmt_free);
     ast_expr_free(a->cond);
FEND

FSTART(ASTForBlock, for)
     ast_lval_free(a->iter);
     mpz_clears(a->from, a->to, NULL);
     FARRAY(a, stmts, ast_stmt_free);
FEND

FSTART(ASTVarDecl, decl)
     ast_var_free(a->var);
     ast_type_free(a->type);
FEND

FSTART(ASTSubParam, param)
     ast_decl_free(a->decl);
FEND

FSTART(ASTStmt, stmt)
     switch (a->kind) {
     case AST_STMT_ASSIGNMENT:
         ast_lval_free(a->lval);
         ast_expr_free(a->rval);
         break;
     case AST_STMT_EXPR:
         ast_expr_free(a->expr);
         break;
     case AST_STMT_IF:
         ast_if_free(a->ifb);
         break;
     case AST_STMT_WHILE:
         ast_while_free(a->whileb);
         break;
     case AST_STMT_DO_WHILE:
         ast_dowhile_free(a->dowhileb);
         break;
     case AST_STMT_FOR:
         ast_for_free(a->forb);
         break;
     }
FEND

FSTART(ASTSub, sub)
     free(a->name);
     FARRAY(a, params, ast_param_free);
     if (a->kind == AST_SUB_FUNC)
         ast_type_free(a->rtype);
     FARRAY(a, vars, ast_decl_free);
     FARRAY(a, stmts, ast_stmt_free);
FEND

void ast_free(ASTProg *ast) {
    assert(ast != NULL);
    free(ast->name);
    FARRAY(ast, subs, ast_sub_free);
    FARRAY(ast, vars, ast_decl_free);
    FARRAY(ast, stmts, ast_stmt_free);
    free(ast);
}
