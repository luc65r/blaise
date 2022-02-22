#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <gmp.h>

#include "ast.h"
#include "ir.h"
#include "utils.h"

IRType ast_type_to_ir(ASTType *ast) {
    if (!strcmp("entier", ast->name))
        return IR_TY_INT;
    if (!strcmp("chaîne", ast->name))
        return IR_TY_STR;
    if (!strcmp("booléen", ast->name))
        return IR_TY_STR;
    unreachable();
}

IRVar *ast_sub_param_to_ir(ASTSubParam *ast) {
    IRVar *ir = malloc(sizeof *ir);
    assert(ir != NULL);
    ir->name = ast->decl->var->name;
    ir->ty = ast_type_to_ir(ast->decl->type);
    return ir;
}

IRVar *ast_var_to_ir(ASTVarDecl *ast) {
    IRVar *ir = malloc(sizeof *ir);
    assert(ir != NULL);
    ir->name = ast->var->name;
    ir->ty = ast_type_to_ir(ast->type);
    return ir;
}

IRExpr *ast_expr_to_ir(IR *ir, IRSub *sub, ASTExpr *astexpr, IRType expected_ty) {
    IRExpr *irexpr = malloc(sizeof *irexpr);
    assert(irexpr != NULL);

    IRType actual_ty;

    switch (astexpr->kind) {
    case AST_EXPR_INTLIT:
        irexpr->kind = IR_EXPR_INTLIT;
        if (!mpz_fits_slong_p(astexpr->intlit))
            unreachable();
        irexpr->intlit = mpz_get_si(astexpr->intlit);
        actual_ty = IR_TY_INT;
        break;
    case AST_EXPR_STRLIT:
        irexpr->kind = IR_EXPR_STRLIT;
        irexpr->strlit = astexpr->strlit;
        actual_ty = IR_TY_STR;
        break;
    case AST_EXPR_BOOLLIT:
        irexpr->kind = IR_EXPR_BOOLLIT;
        irexpr->boollit = astexpr->boollit;
        actual_ty = IR_TY_BOOL;
        break;
    case AST_EXPR_VAR:
        irexpr->kind = IR_EXPR_VAR;
        {
            IRVar *var = map_get(sub->vars, astexpr->var->name);
            if (var == NULL)
                unreachable();
            irexpr->var = var;
            actual_ty = var->ty;
        }
        break;
    case AST_EXPR_BINEXPR:
        TODO;
        break;
    case AST_EXPR_CALL:
        TODO;
        break;
    }

    if (actual_ty != expected_ty) {
        TODO;
    }

    return irexpr;
}

IRStmt *ast_stmt_to_ir(IR *ir, IRSub *sub, ASTStmt *aststmt) {
    IRStmt *irstmt = malloc(sizeof *irstmt);
    assert(irstmt != NULL);

    switch (aststmt->kind) {
    case AST_STMT_ASSIGNMENT:
        irstmt->kind = IR_STMT_ASSIGNMENT;
        irstmt->lval = malloc(sizeof *irstmt->lval);
        assert(irstmt->lval != NULL);
        irstmt->lval->kind = IR_LVAL_VAR;
        {
            IRVar *var = map_get(sub->vars, aststmt->lval->var->name);
            if (var == NULL)
                unreachable();
            irstmt->lval->var = var;

            irstmt->rval = ast_expr_to_ir(ir, sub, aststmt->rval, var->ty);
        }
        break;
    case AST_STMT_EXPR:
        irstmt->kind = IR_STMT_EXPR;
        irstmt->expr = ast_expr_to_ir(ir, sub, aststmt->expr, IR_TY_VOID);
        break;
    case AST_STMT_IF:
        TODO;
        break;
    case AST_STMT_WHILE:
        TODO;
        break;
    case AST_STMT_DO_WHILE:
        TODO;
        break;
    case AST_STMT_FOR:
        TODO;
        break;
    case AST_STMT_RETURN:
        TODO;
        break;
    }

    return irstmt;
}

void ast_sub_to_ir(IR *ir, ASTSub *astsub, IRSub *sub) {
    sub->vars = map_init();
    for (size_t i = 0; i < sub->nargs; i++)
        map_insert(sub->vars, sub->args[i]->name, sub->args[i]);
    for (size_t i = 0; i < astsub->nvars; i++)
        map_insert(sub->vars, astsub->vars[i]->var->name, ast_var_to_ir(astsub->vars[i]));

    sub->nstmts = astsub->nstmts;
    sub->stmts = malloc(sizeof *sub->stmts * sub->nstmts);
    assert(sub->stmts != NULL);
    for (size_t i = 0; i < sub->nstmts; i++)
        sub->stmts[i] = ast_stmt_to_ir(ir, sub, astsub->stmts[i]);
}

void ast_main_to_ir(IR *ir, ASTProg *astprog, IRSub *sub) {
    sub->vars = map_init();
    for (size_t i = 0; i < astprog->nvars; i++)
        map_insert(sub->vars, astprog->vars[i]->var->name, ast_var_to_ir(astprog->vars[i]));

    sub->nstmts = astprog->nstmts;
    sub->stmts = malloc(sizeof *sub->stmts * sub->nstmts);
    assert(sub->stmts != NULL);
    for (size_t i = 0; i < sub->nstmts; i++)
        sub->stmts[i] = ast_stmt_to_ir(ir, sub, astprog->stmts[i]);
}

IR *ast_to_ir(ASTProg *ast) {
    IR *ir = malloc(sizeof *ir);
    assert(ir != NULL);

    ir->subs = map_init();

    {
        IRSub *main = calloc(1, sizeof *main);
        assert(main != NULL);
        main->name = strdup("");
        main->rty = IR_TY_VOID;
        map_insert(ir->subs, main->name, main);
    }
    
    for (size_t i = 0; i < ast->nsubs; i++) {
        ASTSub *astsub = ast->subs[i];
        IRSub *sub = calloc(1, sizeof *sub);
        assert(sub != NULL);
        sub->name = astsub->name;
        sub->nargs = astsub->nparams;
        sub->args = malloc(sizeof *sub->args * sub->nargs);
        for (size_t j = 0; j < sub->nargs; j++)
            sub->args[j] = ast_sub_param_to_ir(astsub->params[j]);
        sub->rty = astsub->kind == AST_SUB_FUNC
            ? ast_type_to_ir(astsub->rtype) : IR_TY_VOID;
        map_insert(ir->subs, sub->name, sub);
    }

    ast_main_to_ir(ir, ast, map_get(ir->subs, ""));

    for (size_t i = 0; i < ast->nsubs; i++)
        ast_sub_to_ir(ir, ast->subs[i], map_get(ir->subs, ast->subs[i]->name));

    return ir;
}
