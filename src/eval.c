#include <assert.h>
#include <gmp.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"
#include "eval.h"
#include "map.h"
#include "utils.h"

typedef enum {
    TNONE,
    TINT,
    TBOOL,
    TSTR,
} Type;

typedef struct {
    Type ty;
    union {
        mpz_t i;
        bool b;
        char *s;
    };
} Value;

typedef enum {
    SUB_BUILTIN,
    SUB_USER,
} SubroutineKind;

typedef struct {
    SubroutineKind kind;
    Type rty;
    size_t nargs;
    Type **args;
    union {
        Value (*fn)(size_t nargs, Value ** args);
        ASTSub *sub;
    };
} Subroutine;

typedef struct {
    Map /* Subroutine */ *subs;
    Map /* Value */ *vars;
    Map /* Type */ *types;
} State;

static Value builtin_write(size_t nargs, Value **args);
static void insert_builtin_subs(State *s);
static void insert_builtin_types(State *s);
static Value call_sub(State *s, Subroutine *sub, size_t nargs, Value **args);
static Value cast(Value v, Type t);
static void insert_vars(State *s, size_t ndecls, ASTVarDecl **decls);
static Value eval_expr(State *s, ASTExpr *expr, Type expected);
static void eval_stmt(State *s, ASTStmt *stmt);
static void eval_stmts(State *s, size_t nstmts, ASTStmt **stmts);

static Value builtin_write(size_t nargs, Value **args) {
    assert(nargs == 1);
    assert(args[0]->ty == TSTR);

    printf("%s\n", args[0]->s);

    return (Value){ TNONE };
}

static void insert_builtin_subs(State *s) {
    struct {
        char *n;
        Subroutine sub;
    } subs[] = {
        { "écrire", { SUB_BUILTIN, TNONE, 1, ({
                        Type *t = malloc(sizeof *t);
                        assert(t != NULL);
                        *t = TSTR;
                        Type **v = malloc(sizeof *v);
                        assert(v != NULL);
                        v[0] = t;
                        v;
                    }), .fn = &builtin_write } },
    };

    for (size_t i = 0; i < sizeof subs / sizeof *subs; i++) {
        Subroutine *sub = malloc(sizeof *sub);
        assert(sub != NULL);
        *sub = subs[i].sub;
        map_insert(s->subs, subs[i].n, sub);
    }
}

static void insert_builtin_types(State *s) {
    struct {
        char *n;
        Type t;
    } types[] = {
        { "Entier", TINT },
        { "Chaîne", TSTR },
    };

    for (size_t i = 0; i < sizeof types / sizeof *types; i++) {
        Type *t = malloc(sizeof *t);
        assert(t != NULL);
        *t = types[i].t;
        map_insert(s->types, types[i].n, t);
    }
}

static Value cast(Value v, Type t) {
    switch (v.ty) {
    case TINT:
        switch (t) {
        case TSTR:
            v.s = mpz_get_str(NULL, 10, v.i);
            break;
        default:
            ;
        }
        break;

    case TSTR:
        switch (t) {
        case TINT:
            mpz_init_set_str(v.i, v.s, 10);
            break;
        default:
            ;
        }
        break;

    default:
        ;
    }

    v.ty = t;
    return v;
}

static void insert_vars(State *s, size_t ndecls, ASTVarDecl **decls) {
    for (size_t i = 0; i < ndecls; i++) {
        Value *v = malloc(sizeof *v);
        assert(v != NULL);
        Type *t = map_get(s->types, decls[i]->type->name);
        assert(t != NULL);
        v->ty = *t;
        map_insert(s->vars, decls[i]->var->name, v);
    }
}

static Value eval_expr(State *s, ASTExpr *expr, Type expected) {
    Value v;

    switch (expr->kind) {
    case AST_EXPR_INTLIT:
        v.ty = TINT;
        mpz_init_set(v.i, expr->intlit);
        break;

    case AST_EXPR_STRLIT:
        v.ty = TSTR;
        v.s = expr->strlit;
        break;

    case AST_EXPR_VAR:
        {
            Value *var = map_get(s->vars, expr->var->name);
            assert(var != NULL);
            v = *var;
        }
        break;

    case AST_EXPR_BINEXPR:
        {
            ASTBinExpr *b = expr->binexpr;
            switch (expected) {
            case TINT:
                {
                    void (*fns[])(mpz_t, const mpz_t, const mpz_t) = {
                        [AST_OP_ADD] = &mpz_add,
                        [AST_OP_SUB] = &mpz_sub,
                        [AST_OP_MUL] = &mpz_mul,
                        [AST_OP_DIV] = &mpz_fdiv_q,
                        [AST_OP_MOD] = &mpz_fdiv_r,
                    };
                    Value v1 = eval_expr(s, b->left, TINT);
                    assert(v1.ty == TINT);
                   Value v2 = eval_expr(s, b->right, TINT);
                    assert(v2.ty == TINT);
                    v.ty = TINT;
                    mpz_init(v.i);
                    fns[b->op](v.i, v1.i, v2.i);
                }
                break;
            case TSTR:
                {
                    assert(b->op == AST_OP_ADD);
                    Value v1 = eval_expr(s, b->left, TSTR);
                    assert(v1.ty == TSTR);
                    Value v2 = eval_expr(s, b->right, TSTR);
                    assert(v2.ty == TSTR);
                    size_t l1 = strlen(v1.s);
                    size_t l2 = strlen(v2.s);
                    v.ty = TSTR;
                    v.s = malloc(l1 + l2 + 1);
                    memcpy(v.s, v1.s, l1);
                    memcpy(v.s + l1, v2.s, l2);
                    v.s[l1 + l2] = '\0';
                }
                break;
            default:
                TODO;
            }
        }
        break;

    case AST_EXPR_CALL:
        {
            Subroutine *sub = map_get(s->subs, expr->subname);
            assert(sub != NULL);
            assert(expr->nargs == sub->nargs);
            size_t nargs = expr->nargs;
            Value **args = malloc(sizeof *args * nargs);
            for (size_t i = 0; i < nargs; i++) {
                Value *val = malloc(sizeof *val);
                assert(val != NULL);
                *val = eval_expr(s, expr->args[i], *sub->args[i]);
                args[i] = val;
            }
            v = call_sub(s, sub, nargs, args);
        }
        break;

    default:
        TODO;
    }

    return cast(v, expected);
}

static void eval_stmt(State *s, ASTStmt *stmt) {
    switch (stmt->kind) {
    case AST_STMT_ASSIGNMENT:
        {
            Value *var = map_get(s->vars, stmt->lval->var->name);
            assert(var != NULL);
            *var = eval_expr(s, stmt->rval, var->ty);
        }
        break;

    case AST_STMT_EXPR:
        eval_expr(s, stmt->expr, TNONE);
        break;

    default:
        TODO;
    }
}

static void eval_stmts(State *s, size_t nstmts, ASTStmt **stmts) {
    for (size_t i = 0; i < nstmts; i++)
        eval_stmt(s, stmts[i]);
}

static Value call_sub(State *s, Subroutine *sub, size_t nargs, Value **args) {
    assert(nargs == sub->nargs);
    switch (sub->kind) {
    case SUB_BUILTIN:
        sub->fn(nargs, args);
        break;

    case SUB_USER:
        for (size_t i = 0; i < nargs; i++)
            map_insert(s->vars, sub->sub->params[i]->decl->var->name, args[i]);
        eval_stmts(s, sub->sub->nstmts, sub->sub->stmts);
        break;
    }

    return (Value){ TNONE };
}

void eval(ASTProg *ast) {
    State s;
    s.subs = map_init();
    s.vars = map_init();
    s.types = map_init();

    insert_builtin_types(&s);
    insert_builtin_subs(&s);

    insert_vars(&s, ast->nvars, ast->vars);
    eval_stmts(&s, ast->nstmts, ast->stmts);
}
