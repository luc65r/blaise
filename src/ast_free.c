#include <assert.h>
#include <gmp.h>
#include <stdlib.h>

#include "ast.h"
#include "utils.h"

#define FSTART(T, A)                            \
    void ast_ ## A ## _free(T *a) {             \
    assert(a != NULL);
#define FEND                                    \
    free(a);                                    \
    }

#define FREE(X)                                 \
    ast_free(a->X)

#define FARRAY(X) do {                              \
        for (size_t _i = 0; _i < a->n ## X; _i++) { \
            ast_free(a->X[_i]);                     \
        }                                           \
        free(a->X);                                 \
    } while (0)

#define F(X)                                    \
    FREE(X);
#define FA(X)                                   \
    FARRAY(X);

FSTART(ASTVar, var)
     FREE(name);
FEND

FSTART(ASTType, type)
     FREE(name);
FEND

FSTART(ASTBinExpr, binexpr)
     FOR_EACH(F, left, right);
FEND

FSTART(ASTExpr, expr)
     switch (a->kind) {
     case AST_EXPR_INTLIT:
         mpz_clear(a->intlit);
         break;
     case AST_EXPR_STRLIT:
         FREE(strlit);
         break;
     case AST_EXPR_BOOLLIT:
         break;
     case AST_EXPR_VAR:
         FREE(var);
         break;
     case AST_EXPR_BINEXPR:
         FREE(binexpr);
         break;
     case AST_EXPR_CALL:
         FREE(subname);
         FARRAY(args);
         break;
     }
FEND

FSTART(ASTLval, lval)
     switch (a->kind) {
     case AST_LVAL_VAR:
         FREE(var);
         break;
     }
FEND

FSTART(ASTIfElseBlock, if)
     if (a->cond != NULL)
         FREE(cond);
     FARRAY(stmts);
     if (a->elseb != NULL)
         FREE(elseb);
FEND

FSTART(ASTWhileBlock, while)
     FREE(cond);
     FARRAY(stmts);
FEND

FSTART(ASTDoWhileBlock, dowhile)
     FARRAY(stmts);
     FREE(cond);
FEND

FSTART(ASTForBlock, for)
     FREE(iter);
     FREE(from);
     FREE(to);
     FARRAY(stmts);
FEND

FSTART(ASTVarDecl, decl)
     FOR_EACH(F, var, type);
FEND

FSTART(ASTSubParam, param)
     FREE(decl);
FEND

FSTART(ASTStmt, stmt)
     switch (a->kind) {
     case AST_STMT_ASSIGNMENT:
         FOR_EACH(F, lval, rval);
         break;
     case AST_STMT_EXPR:
         FREE(expr);
         break;
     case AST_STMT_IF:
         FREE(ifb);
         break;
     case AST_STMT_WHILE:
         FREE(whileb);
         break;
     case AST_STMT_DO_WHILE:
         FREE(dowhileb);
         break;
     case AST_STMT_FOR:
         FREE(forb);
         break;
     case AST_STMT_RETURN:
         FREE(expr);
         break;
     }
FEND

FSTART(ASTSub, sub)
     FREE(name);
     FARRAY(params);
     if (a->kind == AST_SUB_FUNC)
         FREE(rtype);
     FOR_EACH(FA, vars, stmts);
FEND

FSTART(ASTProg, prog)
    FREE(name);
    FOR_EACH(FA, subs, vars, stmts);
FEND
