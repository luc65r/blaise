#include <assert.h>
#include <gmp.h>
#include <jansson.h>
#include <stdio.h>
#include <stdlib.h>

#include "ast.h"
#include "utils.h"

#define COMPACT_LOC 1

#define ARRAY(N, E, F) ({                                           \
            size_t _n = (N);                                        \
            __auto_type _e = (E);                                   \
            json_t *_ja = json_array();                             \
            for (size_t _i = 0; _i < _n; _i++) {                    \
                int _err = json_array_append_new(_ja, F(_e[_i]));   \
                assert(_err == 0);                                  \
            }                                                       \
            _ja;                                                    \
        })

static json_t *ast_loc_json(ASTLoc loc);
static json_t *ast_sub_json(ASTSub *sub);
static json_t *ast_decl_json(ASTVarDecl *decl);
static json_t *ast_stmt_json(ASTStmt *stmt);
static json_t *ast_lval_json(ASTLval *lval);
static json_t *ast_expr_json(ASTExpr *expr);
static json_t *ast_binexpr_json(ASTBinExpr *expr);
static json_t *ast_if_json(ASTIfElseBlock *b);
static json_t *ast_while_json(ASTWhileBlock *b);
static json_t *ast_dowhile_json(ASTDoWhileBlock *b);
static json_t *ast_for_json(ASTForBlock *b);
static json_t *ast_sub_param_json(ASTSubParam *param);
static json_t *ast_type_json(ASTType *type);
static json_t *ast_var_json(ASTVar *var);

json_t *ast_json(ASTProg *ast) {
    assert(ast != NULL);
    return json_pack("{s:o, s:s, s:o, s:o, s:o}",
                     "loc", ast_loc_json(ast->loc),
                     "name", ast->name,
                     "subs", ARRAY(ast->nsubs, ast->subs, ast_sub_json),
                     "vars", ARRAY(ast->nvars, ast->vars, ast_decl_json),
                     "stmts", ARRAY(ast->nstmts, ast->stmts, ast_stmt_json));
}

static json_t *ast_loc_json(ASTLoc loc) {
#if COMPACT_LOC
    return json_sprintf("%zu,%zu-%zu,%zu", loc.sl, loc.sc, loc.el, loc.ec);
#else
#define F(X) #X, loc.X
    return json_pack("{s:i, s:i, s:i, s:i}", F(sl), F(sc), F(el), F(ec));
#undef F
#endif
}

static json_t *ast_sub_json(ASTSub *sub) {
    assert(sub != NULL);
    char *kind[] = {
        [AST_SUB_FUNC] = "function",
        [AST_SUB_PROC] = "procedure",
    };
    json_t *j = json_pack("{s:o, s:s, s:s, s:o, s:o, s:o}",
                          "loc", ast_loc_json(sub->loc),
                          "name", sub->name,
                          "kind", kind[sub->kind],
                          "params", ARRAY(sub->nparams, sub->params, ast_sub_param_json),
                          "vars", ARRAY(sub->nvars, sub->vars, ast_decl_json),
                          "stmts", ARRAY(sub->nstmts, sub->stmts, ast_stmt_json));
    if (sub->kind == AST_SUB_FUNC) {
        int err = json_object_set_new(j, "rtype", ast_type_json(sub->rtype));
        assert(err == 0);
    }
    return j;
}

static json_t *ast_decl_json(ASTVarDecl *decl) {
    assert(decl != NULL);
    return json_pack("{s:o, s:o, s:o}",
                     "loc", ast_loc_json(decl->loc),
                     "var", ast_var_json(decl->var),
                     "type", ast_type_json(decl->type));
}

static json_t *ast_stmt_json(ASTStmt *stmt) {
    assert(stmt != NULL);
    json_t *j = json_pack("{s:o}", "loc", ast_loc_json(stmt->loc));
    switch (stmt->kind) {
    case AST_STMT_ASSIGNMENT:
        json_object_set_new(j, "lval", ast_lval_json(stmt->lval));
        json_object_set_new(j, "rval", ast_expr_json(stmt->rval));
        break;
    case AST_STMT_EXPR:
        json_object_set_new(j, "expr", ast_expr_json(stmt->expr));
        break;
    case AST_STMT_IF:
        json_object_set_new(j, "if", ast_if_json(stmt->ifb));
        break;
    case AST_STMT_WHILE:
        json_object_set_new(j, "while", ast_while_json(stmt->whileb));
        break;
    case AST_STMT_DO_WHILE:
        json_object_set_new(j, "do while", ast_dowhile_json(stmt->dowhileb));
        break;
    case AST_STMT_FOR:
        json_object_set_new(j, "for", ast_for_json(stmt->forb));
        break;
    }
    return j;
}

static json_t *ast_lval_json(ASTLval *lval) {
    assert(lval != NULL);
    return json_pack("{s:o, s:o}",
                     "loc", ast_loc_json(lval->loc),
                     "var", ast_var_json(lval->var));
}

static json_t *ast_expr_json(ASTExpr *expr) {
    assert(expr != NULL);
    json_t *j = json_pack("{s:o}", "loc", ast_loc_json(expr->loc));
    switch (expr->kind) {
    case AST_EXPR_INTLIT:
        json_object_set_new(j, "intlit", json_string(mpz_get_str(NULL, 10, expr->intlit)));
        break;
    case AST_EXPR_STRLIT:
        json_object_set_new(j, "strlit", json_string(expr->strlit));
        break;
    case AST_EXPR_BOOLLIT:
        json_object_set_new(j, "boollit", json_boolean(expr->boollit));
        break;
    case AST_EXPR_VAR:
        json_object_set_new(j, "var", ast_var_json(expr->var));
        break;
    case AST_EXPR_BINEXPR:
        json_object_set_new(j, "binexpr", ast_binexpr_json(expr->binexpr));
        break;
    case AST_EXPR_CALL:
        json_object_set_new(j, "subname", json_string(expr->subname));
        json_object_set_new(j, "args", ARRAY(expr->nargs, expr->args, ast_expr_json));
        break;
    }
    return j;
}

static json_t *ast_binexpr_json(ASTBinExpr *expr) {
    assert(expr != NULL);
    char *op[] = {
        [AST_OP_ADD] = "+",
        [AST_OP_SUB] = "-",
        [AST_OP_MUL] = "*",
        [AST_OP_DIV] = "div",
        [AST_OP_MOD] = "mod",
        [AST_OP_AND] = "and",
        [AST_OP_OR] = "or",
        [AST_OP_EQ] = "=",
        [AST_OP_NEQ] = "!=",
        [AST_OP_GT] = ">",
        [AST_OP_LT] = "<",
        [AST_OP_GE] = ">=",
        [AST_OP_LE] = "<=",
    };
    return json_pack("{s:o, s:s, s:o, s:o}",
                     "loc", ast_loc_json(expr->loc),
                     "op", op[expr->op],
                     "left", ast_expr_json(expr->left),
                     "right", ast_expr_json(expr->right));
}

static json_t *ast_if_json(ASTIfElseBlock *b) {
    assert(b != NULL);
    return json_pack("{s:o}", "loc", ast_loc_json(b->loc));
}

static json_t *ast_while_json(ASTWhileBlock *b) {
    assert(b != NULL);
    return json_pack("{s:o}", "loc", ast_loc_json(b->loc));
}

static json_t *ast_dowhile_json(ASTDoWhileBlock *b) {
    assert(b != NULL);
    return json_pack("{s:o}", "loc", ast_loc_json(b->loc));
}

static json_t *ast_for_json(ASTForBlock *b) {
    assert(b != NULL);
    return json_pack("{s:o}", "loc", ast_loc_json(b->loc));
}


static json_t *ast_sub_param_json(ASTSubParam *param) {
    assert(param != NULL);
    char *kind[] = {
        [AST_PARAM_IN] = "in",
        [AST_PARAM_OUT] = "out",
        [AST_PARAM_IN_OUT] = "in out",
    };
    return json_pack("{s:o, s:s, s:o}",
                     "loc", ast_loc_json(param->loc),
                     "kind", kind[param->kind],
                     "decl", ast_decl_json(param->decl));
}

static json_t *ast_type_json(ASTType *type) {
    assert(type != NULL);
    return json_pack("{s:o, s:s}", "loc", ast_loc_json(type->loc), "name", type->name);
}

static json_t *ast_var_json(ASTVar *var) {
    assert(var != NULL);
    return json_pack("{s:o, s:s}", "loc", ast_loc_json(var->loc), "name", var->name);
}
