#pragma once

#include "error.h"
#include <gmp.h>
#include <jansson.h>
#include <stdbool.h>
#include <stddef.h>

#define AST_STRUCTS(X) \
    X(ASTVar, var) X(ASTType, type) X(ASTBinExpr, binexpr) X(ASTExpr, expr) X(ASTLval, lval) X(ASTStmt, stmt) \
    X(ASTIfElseBlock, if) X(ASTWhileBlock, while) X(ASTDoWhileBlock, dowhile) X(ASTForBlock, for) \
    X(ASTVarDecl, decl) X(ASTSubParam, param) X(ASTSub, sub) X(ASTProg, prog)

#define X(T, N) \
    typedef struct T T;
AST_STRUCTS(X)
#undef X

struct ASTVar {
    Loc loc;
    char *name;
};

struct ASTType {
    Loc loc;
    char *name;
};

typedef enum {
    AST_OP_ADD,
    AST_OP_SUB,
    AST_OP_MUL,
    AST_OP_DIV,
    AST_OP_MOD,
    AST_OP_AND,
    AST_OP_OR,
    AST_OP_EQ,
    AST_OP_NEQ,
    AST_OP_GT,
    AST_OP_LT,
    AST_OP_GE,
    AST_OP_LE,
} ASTBinOp;

struct ASTBinExpr {
    Loc loc;
    ASTBinOp op;
    ASTExpr *left, *right;
};

typedef enum {
    AST_EXPR_INTLIT,
    AST_EXPR_STRLIT,
    AST_EXPR_BOOLLIT,
    AST_EXPR_VAR,
    AST_EXPR_BINEXPR,
    AST_EXPR_CALL,
} ASTExprKind;

struct ASTExpr {
    Loc loc;

    ASTExprKind kind;
    union {
        mpz_t intlit;

        char *strlit;

        bool boollit;

        ASTVar *var;

        ASTBinExpr *binexpr;

        struct {
            char *subname;
            size_t nargs;
            ASTExpr **args;
        };
    };
};

typedef enum {
    AST_LVAL_VAR,
} ASTLvalKind;

struct ASTLval {
    Loc loc;

    ASTLvalKind kind;
    union {
        ASTVar *var;
    };
};

struct ASTIfElseBlock {
    Loc loc;

    // NULL if no condition
    ASTExpr *cond;

    size_t nstmts;
    ASTStmt **stmts;

    // NULL if simple else;
    ASTIfElseBlock *elseb;
};

struct ASTWhileBlock {
    Loc loc;

    ASTExpr *cond;

    size_t nstmts;
    ASTStmt **stmts;
};

struct ASTDoWhileBlock {
    Loc loc;

    size_t nstmts;
    ASTStmt **stmts;

    ASTExpr *cond;
};

struct ASTForBlock {
    Loc loc;

    ASTLval *iter;
    ASTExpr *from;
    ASTExpr *to;

    size_t nstmts;
    ASTStmt **stmts;
};

typedef enum {
    AST_STMT_ASSIGNMENT,
    AST_STMT_EXPR,
    AST_STMT_IF,
    AST_STMT_WHILE,
    AST_STMT_DO_WHILE,
    AST_STMT_FOR,
    AST_STMT_RETURN,
} ASTStmtKind;

struct ASTStmt {
    Loc loc;

    ASTStmtKind kind;
    union {
        /* AST_STMT_ASSIGNMENT */
        struct {
            ASTLval *lval;
            ASTExpr *rval;
        };

        /* AST_STMT_EXPR
           AST_STMT_RETURN */
        ASTExpr *expr;

        ASTIfElseBlock *ifb;
        ASTWhileBlock *whileb;
        ASTDoWhileBlock *dowhileb;
        ASTForBlock *forb;
    };
};

struct ASTVarDecl {
    Loc loc;

    ASTVar *var;
    ASTType *type;
};

typedef enum {
    AST_PARAM_IN,
    AST_PARAM_OUT,
    AST_PARAM_IN_OUT,
} ASTSubParamKind;

struct ASTSubParam {
    Loc loc;

    ASTSubParamKind kind;
    ASTVarDecl *decl;
};

typedef enum {
    AST_SUB_FUNC,
    AST_SUB_PROC,
} ASTSubKind;

struct ASTSub {
    Loc loc;

    char *name;

    ASTSubKind kind;

    size_t nparams;
    ASTSubParam **params;
    union {
        ASTType *rtype;
    };

    size_t nvars;
    ASTVarDecl **vars;

    size_t nstmts;
    ASTStmt **stmts;
};

struct ASTProg {
    Loc loc;

    char *name;

    size_t nsubs;
    ASTSub **subs;

    size_t nvars;
    ASTVarDecl **vars;

    size_t nstmts;
    ASTStmt **stmts;
};

#define X(T, N) \
    void ast_ ## N ## _free(T *);
AST_STRUCTS(X)
#undef X

#define _AST_FREE_GENERIC(T, N) \
    T *: ast_ ## N ## _free,

#define ast_free(A) \
    _Generic((A), AST_STRUCTS(_AST_FREE_GENERIC) char *: free)(A)

json_t *ast_loc_json(Loc);
#define X(T, N) \
    json_t *ast_ ## N ## _json(T *);
AST_STRUCTS(X)
#undef X

#define _AST_JSON_GENERIC(T, N) \
    T *: ast_ ## N ## _json,

#define ast_json(A) \
    _Generic((A), AST_STRUCTS(_AST_JSON_GENERIC) Loc: ast_loc_json)(A)

//void ast_pretty_print(FILE *f, ASTProg *ast);
