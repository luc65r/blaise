#pragma once

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

typedef struct {
    size_t sl, sc, el, ec;
} ASTLoc;

struct ASTVar {
    ASTLoc loc;
    char *name;
};

struct ASTType {
    ASTLoc loc;
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
    ASTLoc loc;
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
    ASTLoc loc;

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
    ASTLoc loc;

    ASTLvalKind kind;
    union {
        ASTVar *var;
    };
};

struct ASTIfElseBlock {
    ASTLoc loc;

    // NULL if no condition
    ASTExpr *cond;

    size_t nstmts;
    ASTStmt **stmts;

    // NULL if simple else;
    ASTIfElseBlock *elseb;
};

struct ASTWhileBlock {
    ASTLoc loc;

    ASTExpr *cond;

    size_t nstmts;
    ASTStmt **stmts;
};

struct ASTDoWhileBlock {
    ASTLoc loc;

    size_t nstmts;
    ASTStmt **stmts;

    ASTExpr *cond;
};

struct ASTForBlock {
    ASTLoc loc;

    ASTLval *iter;
    mpz_t from;
    mpz_t to;

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
} ASTStmtKind;

struct ASTStmt {
    ASTLoc loc;

    ASTStmtKind kind;
    union {
        struct {
            ASTLval *lval;
            ASTExpr *rval;
        };

        ASTExpr *expr;

        ASTIfElseBlock *ifb;
        ASTWhileBlock *whileb;
        ASTDoWhileBlock *dowhileb;
        ASTForBlock *forb;
    };
};

struct ASTVarDecl {
    ASTLoc loc;

    ASTVar *var;
    ASTType *type;
};

typedef enum {
    AST_PARAM_IN,
    AST_PARAM_OUT,
    AST_PARAM_IN_OUT,
} ASTSubParamKind;

struct ASTSubParam {
    ASTLoc loc;

    ASTSubParamKind kind;
    ASTVarDecl *decl;
};

typedef enum {
    AST_SUB_FUNC,
    AST_SUB_PROC,
} ASTSubKind;

struct ASTSub {
    ASTLoc loc;

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
    ASTLoc loc;

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

//void ast_pretty_print(FILE *f, ASTProg *ast);
json_t *ast_json(ASTProg *ast);
