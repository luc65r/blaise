#pragma once

#include <gmp.h>
#include <stdbool.h>
#include <stddef.h>

typedef struct {
    size_t sl, sc, el, ec;
} ASTLoc;

typedef struct {
    ASTLoc loc;
    char *name;
} ASTVar;

typedef struct {
    ASTLoc loc;
    char *name;
} ASTType;

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

typedef struct ASTExpr ASTExpr;
typedef struct ASTBinExpr ASTBinExpr;

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

typedef struct {
    ASTLoc loc;

    ASTLvalKind kind;
    union {
        ASTVar *var;
    };
} ASTLval;

typedef struct ASTStmt ASTStmt;
typedef struct ASTIfElseBlock ASTIfElseBlock;

struct ASTIfElseBlock {
    ASTLoc loc;

    // NULL if no condition
    ASTExpr *cond;

    size_t nstmts;
    ASTStmt **stmts;

    // NULL if simple else;
    ASTIfElseBlock *elseb;
};

typedef struct {
    ASTLoc loc;

    ASTExpr *cond;

    size_t nstmts;
    ASTStmt **stmts;
} ASTWhileBlock;

typedef struct {
    ASTLoc loc;

    size_t nstmts;
    ASTStmt **stmts;

    ASTExpr *cond;
} ASTDoWhileBlock;

typedef struct {
    ASTLoc loc;

    ASTLval *iter;
    mpz_t from;
    mpz_t to;

    size_t nstmts;
    ASTStmt **stmts;
} ASTForBlock;

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

typedef struct {
    ASTLoc loc;

    ASTVar *var;
    ASTType *type;
} ASTVarDecl;

typedef enum {
    AST_PARAM_IN,
    AST_PARAM_OUT,
    AST_PARAM_IN_OUT,
} ASTSubParamKind;

typedef struct {
    ASTLoc loc;

    ASTSubParamKind kind;
    ASTVarDecl *decl;
} ASTSubParam;

typedef enum {
    AST_SUB_FUNC,
    AST_SUB_PROC,
} ASTSubKind;

typedef struct {
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
} ASTSub;

typedef struct {
    ASTLoc loc;

    char *name;

    size_t nsubs;
    ASTSub **subs;

    size_t nvars;
    ASTVarDecl **vars;

    size_t nstmts;
    ASTStmt **stmts;
} ASTProg;

void ast_dump(ASTProg *ast);
