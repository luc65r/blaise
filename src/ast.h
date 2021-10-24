#pragma once

#include <stddef.h>

#include "vec.h"

typedef struct {
    char *name;
} ASTVariable;

typedef struct {
    char *name;
} ASTType;

typedef enum {
    AST_OP_ADD,
} ASTBinaryOperator;

typedef struct ASTExpression ASTExpression;
typedef struct ASTBinaryExpression ASTBinaryExpression;

struct ASTBinaryExpression {
    ASTBinaryOperator op;
    ASTExpression *left, *right;
};

typedef enum {
    AST_EXPR_VARIABLE,
    AST_EXPR_BINARY_EXPRESSION,
    AST_EXPR_SUBROUTINE_CALL,
} ASTExpressionKind;

struct ASTExpression {
    ASTExpressionKind kind;
    union {
        ASTVariable variable;

        ASTBinaryExpression binary_expression;

        // subroutine call
        struct {
            char *subroutine_name;
            Vec /* ASTExpression */ arguments;
        };
    };
};

typedef enum {
    AST_LVALUE_VARIABLE,
} ASTLvalueKind;

typedef struct {
    ASTLvalueKind kind;
    union {
        ASTVariable variable;
    };
} ASTLvalue;

typedef enum {
    AST_STMT_ASSIGNMENT,
    AST_STMT_EXPRESSION,
} ASTStatementKind;

typedef struct {
    ASTStatementKind kind;
    union {
        // assignment
        struct {
            ASTLvalue lvalue;
            ASTExpression *rvalue;
        };

        ASTExpression *expression;
    };
} ASTStatement;

typedef struct {
    ASTVariable variable;
    ASTType type;
} ASTVariableDecl;

typedef enum {
    AST_PARAM_IN,
    AST_PARAM_OUT,
    AST_PARAM_IN_OUT,
} ASTSubroutineParamaterKind;

typedef struct {
    ASTSubroutineParamaterKind kind;
    ASTVariableDecl decl;
} ASTSubroutineParameter;

typedef enum {
    AST_SUB_FUNCTION,
    AST_SUB_PROCEDURE,
} ASTSubroutineKind;

typedef struct {
    char *name;

    ASTSubroutineKind kind;

    Vec /* ASTSubroutineParameter */ parameters;
    union {
        // function
        ASTType return_type;
    };

    Vec /* ASTVariableDecl */ variables;
    Vec /* ASTStatement */ statements;
} ASTSubroutine;

typedef struct {
    char *name;

    Vec /* ASTSUbroutine */ subroutines;
    Vec /* ASTVariableDecl */ variables;
    Vec /* ASTStatement */ statements;
} ASTProgram;

void ast_dump(ASTProgram *ast);
