#include <assert.h>
#include <gmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"
#include "eval.h"
#include "map.h"
#include "utils.h"
#include "vec.h"

typedef enum {
    TNONE,
    TINT,
    TSTR,
} Type;

typedef struct {
    Type ty;
    union {
        mpz_t i;
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
    Vec /* Type */ args;
    union {
        Value (*fn)(Vec /* Value */);
        ASTSubroutine sub;
    };
} Subroutine;

typedef struct {
    Map /* Subroutine */ *subs;
    Map /* Value */ *vars;
    Map /* Type */ *types;
} State;

static Value builtin_write(Vec vals);
static void insert_builtin_subs(State *s);
static void insert_builtin_types(State *s);
static Value call_sub(State *s, Subroutine *sub, Vec /* Value */ args);
static Value cast(Value v, Type t);
static void insert_vars(State *s, Vec /* ASTVariableDecl */ decls);
static Value eval_expr(State *s, ASTExpression *expr, Type expected);
static void eval_stmt(State *s, ASTStatement *stmt);
static void eval_stmts(State *s, Vec /* ASTStatement */ stmts);

static Value builtin_write(Vec vals) {
    assert(vals.len == 1);
    Value *v = vals.elems[0];
    assert(v->ty == TSTR);

    printf("%s\n", v->s);

    return (Value){ TNONE };
}

static void insert_builtin_subs(State *s) {
    struct {
        char *n;
        Subroutine sub;
    } subs[] = {
        { "écrire", { SUB_BUILTIN, TNONE, ({
                        Type *t = malloc(sizeof *t);
                        assert(t != NULL);
                        *t = TSTR;
                        Vec v = VEC_EMPTY;
                        vec_push(&v, t);
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

static void insert_vars(State *s, Vec /* ASTVariableDecl */ decls) {
    for (size_t i = 0; i < decls.len; i++) {
        ASTVariableDecl *d = decls.elems[i];
        Value *v = malloc(sizeof *v);
        assert(v != NULL);
        Type *t = map_get(s->types, d->type.name);
        assert(t != NULL);
        v->ty = *t;
        map_insert(s->vars, d->variable.name, v);
    }
}

static Value eval_expr(State *s, ASTExpression *expr, Type expected) {
    Value v;

    switch (expr->kind) {
    case AST_EXPR_INTEGER_LITTERAL:
        v.ty = TINT;
        mpz_init_set(v.i, expr->integer_litteral);
        break;

    case AST_EXPR_STRING_LITTERAL:
        v.ty = TSTR;
        v.s = expr->string_litteral;
        break;

    case AST_EXPR_VARIABLE:
        {
            Value *var = map_get(s->vars, expr->variable.name);
            assert(var != NULL);
            v = *var;
        }
        break;

    case AST_EXPR_BINARY_EXPRESSION:
        {
            ASTBinaryExpression *b = &expr->binary_expression;
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
            case TNONE:
                unreachable();
            }
        }
        break;

    case AST_EXPR_SUBROUTINE_CALL:
        {
            Subroutine *sub = map_get(s->subs, expr->subroutine_name);
            assert(sub != NULL);
            assert(expr->arguments.len == sub->args.len);
            Vec /* Value */ args = VEC_EMPTY;
            for (size_t i = 0; i < sub->args.len; i++) {
                Type *t = sub->args.elems[i];
                ASTExpression *e = expr->arguments.elems[i];
                Value *v1 = malloc(sizeof *v1);
                assert(v1 != NULL);
                *v1 = eval_expr(s, e, *t);
                vec_push(&args, v1);
            }
            v = call_sub(s, sub, args);
        }
        break;
    }

    return cast(v, expected);
}

static void eval_stmt(State *s, ASTStatement *stmt) {
    switch (stmt->kind) {
    case AST_STMT_ASSIGNMENT:
        {
            Value *var = map_get(s->vars, stmt->lvalue.variable.name);
            assert(var != NULL);
            *var = eval_expr(s, stmt->rvalue, var->ty);
        }
        break;
        
    case AST_STMT_EXPRESSION:
        eval_expr(s, stmt->expression, TNONE);
        break;
    }
}

static void eval_stmts(State *s, Vec /* ASTStatement */ stmts) {
    for (size_t i = 0; i < stmts.len; i++)
        eval_stmt(s, stmts.elems[i]);
}

static Value call_sub(State *s, Subroutine *sub, Vec /* Value */ args) {
    assert(args.len == sub->args.len);
    switch (sub->kind) {
    case SUB_BUILTIN:
        sub->fn(args);
        break;

    case SUB_USER:
        {
            ASTSubroutine *ast = &sub->sub;
            for (size_t i = 0; i < args.len; i++) {
                ASTSubroutineParameter *p = ast->parameters.elems[i];
                map_insert(s->vars, p->decl.variable.name, args.elems[i]);
            }

            eval_stmts(s, ast->statements);
        }
        break;
    }

    return (Value){ TNONE };
}

void eval(ASTProgram *ast) {
    State s;
    s.subs = map_init();
    s.vars = map_init();
    s.types = map_init();

    insert_builtin_types(&s);
    insert_builtin_subs(&s);

    insert_vars(&s, ast->variables);
    eval_stmts(&s, ast->statements);
}
