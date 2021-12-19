#pragma once

#include "ast.h"
#include "map.h"

typedef enum {
    IR_TY_INT,
    IR_TY_STR,
    IR_TY_BOOL,
} IRType;

typedef enum {
    char *name;
    IRType *ty;
} IRVar;

typedef enum {
    IR_INT_ADD,
    IR_INT_SUB,
    IR_INT_MUL,
    IR_INT_DIV,
    IR_INT_MOD,
    IR_INT_EQ,
    IR_INT_NEQ,
    IR_INT_GT,
    IR_INT_LT,
    IR_INT_GE,
    IR_INT_LE,
    IR_STR_CONCAT,
    IR_STR_EQ,
    IR_STR_NEQ,
    IR_BOOL_AND,
    IR_BOOL_OR,
    IR_INT_TO_STR,
    IR_STR_TO_INT,
    IR_BOOL_TO_INT,
    IR_INT_TO_BOOL,
} IROperation;

typedef enum {
    IR_EXPR_OP,
    IR_EXPR_VAR,
    IR_EXPR_CALL,
} IRExprKind;

typedef struct {
    IRExprKind kind;

    union {
        /* IR_EXPR_OP */
        struct {
            IROperation op;
            IRExpr *lhs, *rhs;
        };

        /* IR_EXPR_VAR */
        IRVar *var;

        /* IR_EXPR_CALL */
        struct {
            IRSub *sub;
            size_t nargs;
            IRExpr **args;
        };
    };
} IRExpr;

typedef struct IRStmt IRStmt;

typedef struct {
    IRExpr *cond;

    size_t nstmts_true;
    IRStmt **stmts_true;

    size_t nstmts_false;
    IRStmt **stmts_false;
} IRIfElse;

typedef enum {
    IR_STMT_ASSIGNMENT,
    IR_STMT_EXPR,
    IR_STMT_RETURN,
    IR_STMT_IF_ELSE,
    IR_STMT_JUMP,
} IRStmtKind;

struct IRStmt {
    IRStmt kind;
    union {
        /* IR_STMT_ASSIGNMENT */
        struct {
            IRLval *lval;
            IRExpr *rval;
        };

        /* IR_STMT_EXPR
           IR_STMT_RETURN */
        IRExpr *expr;

        /* IR_STMT_IF_ELSE */
        IRIfElse *ifelse;

        /* IR_STMT_JUMP */
        size_t stmt;
    };
};

typedef struct {
    char *name;
    Map /* IRVar */ vars;

    size_t nstmts;
    IRStmt **stmts;
} IRSub;

typedef struct {
    Map /* IRSub */ subs;
} IR;

IR *ast_to_ir(ASTProg *ast);
