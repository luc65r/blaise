#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "ast.h"
#include "utils.h"

#define INDENT(N) printf("%*s", (int)((N) * 4), "")

static void ast_pprint_variables(Vec /* ASTVariableDecl */ vars) {
    if (vars.len == 0)
        return;

    printf("VARIABLES\n");
    for (size_t i = 0; i < vars.len; i++) {
        ASTVariableDecl *v = vars.elems[i];
        INDENT(1);
        printf("%s : %s\n", v->variable.name, v->type.name);
    }
}

static void ast_pprint_expression(ASTExpression *expr) {
    assert(expr != NULL);

    switch (expr->kind) {
    case AST_EXPR_VARIABLE:
        printf("%s", expr->variable.name);
        break;

    case AST_EXPR_BINARY_EXPRESSION:
        ast_pprint_expression(expr->binary_expression.left);
        switch (expr->binary_expression.op) {
        case AST_OP_ADD:
            printf(" + ");
            break;
        }
        ast_pprint_expression(expr->binary_expression.right);
        break;

    case AST_EXPR_SUBROUTINE_CALL:
        printf("%s(", expr->subroutine_name);
        for (size_t i = 0; i < expr->arguments.len; i++) {
            if (i != 0)
                printf(", ");
            ast_pprint_expression(expr->arguments.elems[i]);
        }
        printf(")");
        break;
    }
}

static void ast_pprint_statements(Vec /* ASTStatement */ stmts, size_t indent_level) {
    for (size_t i = 0; i < stmts.len; i++) {
        ASTExpression *expr;
        ASTStatement *stmt = stmts.elems[i];

        INDENT(indent_level);

        switch (stmt->kind) {
        case AST_STMT_ASSIGNMENT:
            switch (stmt->lvalue.kind) {
            case AST_LVALUE_VARIABLE:
                printf("%s <- ", stmt->lvalue.variable.name);
                expr = stmt->rvalue;
                break;
            }
            [[fallthrough]];

        case AST_STMT_EXPRESSION:
            expr = stmt->expression;
            ast_pprint_expression(expr);
            printf("\n");
            break;
        }
    }
}

static void ast_pprint_subroutine(ASTSubroutine *sub) {
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

    ast_pprint_variables(sub->variables);

    printf("DÉBUT\n");
    ast_pprint_statements(sub->statements, 1);
    printf("FIN\n");
}

void ast_pretty_print(ASTProgram *ast) {
    assert(ast != NULL);

    printf("PROGRAMME %s\n", ast->name);

    if (ast->subroutines.len > 0)
        printf("\n");
    for (size_t i = 0; i < ast->subroutines.len; i++) {
        ast_pprint_subroutine(ast->subroutines.elems[i]);
        printf("\n");
    }

    ast_pprint_variables(ast->variables);

    printf("DÉBUT\n");
    ast_pprint_statements(ast->statements, 1);
    printf("FIN\n");
}
