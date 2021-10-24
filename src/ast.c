#include <assert.h>
#include <gmp.h>
#include <stdio.h>
#include <stdlib.h>

#include "ast.h"
#include "utils.h"

#define PRETTY 0

#define INDENT(N) printf("%*s", (int)((N) * 4), "")

static void ast_dump_variables(Vec /* ASTVariableDecl */ vars) {
#if PRETTY
    if (vars.len == 0)
        return;
#endif

    printf(
#if PRETTY
        "VARIABLES\n"
#else
        "variables:\n"
#endif
    );
    for (size_t i = 0; i < vars.len; i++) {
        ASTVariableDecl *v = vars.elems[i];
        INDENT(1);
        printf("%s : %s\n", v->variable.name, v->type.name);
    }
}

static void ast_dump_expression(ASTExpression *expr
#if !PRETTY
                                , size_t indent_level
#endif
                                ) {
    assert(expr != NULL);

    switch (expr->kind) {
    case AST_EXPR_INTEGER_LITTERAL:
#if PRETTY
        mpz_out_str(stdout, 10, expr->integer_litteral);
#else
        INDENT(indent_level);
        printf("integer: ");
        mpz_out_str(stdout, 10, expr->integer_litteral);
        printf("\n");
#endif
        break;

    case AST_EXPR_VARIABLE:
#if PRETTY
        printf("%s", expr->variable.name);
#else
        INDENT(indent_level);
        printf("variable: %s\n", expr->variable.name);
#endif
        break;

    case AST_EXPR_BINARY_EXPRESSION:
#if PRETTY
        ast_dump_expression(expr->binary_expression.left);
#else
        INDENT(indent_level);
        printf("binary expression:\n");
        INDENT(indent_level + 1);
        printf("left:\n");
        ast_dump_expression(expr->binary_expression.left, indent_level + 2);
        INDENT(indent_level + 1);
        printf("op:");
#endif
        switch (expr->binary_expression.op) {
        case AST_OP_ADD:
            printf(" + ");
            break;
        }

#if PRETTY
        ast_dump_expression(expr->binary_expression.right);
#else
        printf("\n");
        INDENT(indent_level + 1);
        printf("right:\n");
        ast_dump_expression(expr->binary_expression.right, indent_level + 2);
#endif
        break;

    case AST_EXPR_SUBROUTINE_CALL:
#if PRETTY
        printf("%s(", expr->subroutine_name);
#else
        INDENT(indent_level);
        printf("subroutine call: %s\n", expr->subroutine_name);
#endif
        for (size_t i = 0; i < expr->arguments.len; i++) {
#if PRETTY
            if (i != 0)
                printf(", ");
            ast_dump_expression(expr->arguments.elems[i]);
#else
            ast_dump_expression(expr->arguments.elems[i], indent_level + 1);
#endif
        }
#if PRETTY
        printf(")");
#endif
        break;
    }
}

static void ast_dump_statements(Vec /* ASTStatement */ stmts, size_t indent_level) {
    for (size_t i = 0; i < stmts.len; i++) {
        ASTStatement *stmt = stmts.elems[i];
        ASTExpression *expr = stmt->expression;

        INDENT(indent_level);

        switch (stmt->kind) {
        case AST_STMT_ASSIGNMENT:
#if !PRETTY
            printf("assignment:\n");
            INDENT(++indent_level);
            printf("lvalue:\n");
#endif
            switch (stmt->lvalue.kind) {
            case AST_LVALUE_VARIABLE:
#if PRETTY
                printf("%s <- ", stmt->lvalue.variable.name);
#else
                INDENT(indent_level + 1);
                printf("variable: %s\n", stmt->lvalue.variable.name);
#endif
                break;
            }
            expr = stmt->rvalue;
            [[fallthrough]];

        case AST_STMT_EXPRESSION:
#if PRETTY
            ast_dump_expression(expr);
            printf("\n");
#else
            INDENT(indent_level);
            printf("rvalue:\n");
            ast_dump_expression(expr, indent_level + 1);
#endif
            break;
        }
    }
}

static void ast_dump_subroutine(ASTSubroutine *sub) {
    assert(sub != NULL);

    switch (sub->kind) {
    case AST_SUB_FUNCTION:
        printf("FONCTION %s(", sub->name);
        for (size_t i = 0; i < sub->parameters.len; i++) {
            if (i != 0)
                printf(", ");
            ASTSubroutineParameter *param = sub->parameters.elems[i];
            printf("%s : %s", param->decl.variable.name, param->decl.type.name);
        }
        printf("): %s\n", sub->return_type.name);
        break;

    case AST_SUB_PROCEDURE:
        printf("PROCÉDURE %s(", sub->name);
        for (size_t i = 0; i < sub->parameters.len; i++) {
            if (i != 0)
                printf(", ");
            ASTSubroutineParameter *param = sub->parameters.elems[i];
            switch (param->kind) {
            case AST_PARAM_IN:
                printf("E ");
                break;
            case AST_PARAM_OUT:
                printf("S ");
                break;
            case AST_PARAM_IN_OUT:
                printf("ES ");
                break;
            }
            printf("%s : %s", param->decl.variable.name, param->decl.type.name);
        }
        printf(")\n");
        break;
    }

    ast_dump_variables(sub->variables);

    printf("DÉBUT\n");
    ast_dump_statements(sub->statements, 1);
    printf("FIN\n");
}

void ast_dump(ASTProgram *ast) {
    assert(ast != NULL);

    printf(
#if PRETTY
           "PROGRAMME"
#else
           "program:"
#endif
           " %s\n", ast->name);

#if PRETTY
    if (ast->subroutines.len > 0)
        printf("\n");
#else
    printf("subroutines:\n");
#endif
    for (size_t i = 0; i < ast->subroutines.len; i++) {
        ast_dump_subroutine(ast->subroutines.elems[i]);
        printf("\n");
    }

    ast_dump_variables(ast->variables);

#if PRETTY
    printf("DÉBUT\n");
#else
    printf("statements:\n");
#endif
    ast_dump_statements(ast->statements, 1);
#if PRETTY
    printf("FIN\n");
#endif
}
