#include <assert.h>
#include <stdlib.h>
#include <llvm-c/Core.h>

#include "ir.h"
#include "map.h"
#include "utils.h"

static LLVMModuleRef module;
static LLVMBuilderRef builder;

LLVMTypeRef codegen_type(IRType ty) {
    switch (ty) {
    case IR_TY_VOID:
        return LLVMVoidType();
    case IR_TY_INT:
        return LLVMInt64Type();
    case IR_TY_STR:
        TODO;
    case IR_TY_BOOL:
        return LLVMInt1Type();
    }

    return NULL;
}

LLVMTypeRef codegen_sub_type(IRSub *sub) {
    LLVMTypeRef *args = malloc(sizeof *args * sub->nargs);
    assert(args != NULL);
    for (size_t i = 0; i < sub->nargs; i++)
        args[i] = codegen_type(sub->args[i]->ty);
    return LLVMFunctionType(codegen_type(sub->rty), args, sub->nargs, 0);
}

void codegen_sub(IRSub *sub) {
    LLVMValueRef fun = LLVMAddFunction(module, sub->name, codegen_sub_type(sub));
    LLVMBasicBlockRef entry = LLVMAppendBasicBlock(fun, "entry");
    LLVMPositionBuilderAtEnd(builder, entry);
}

LLVMModuleRef codegen(IR *ir) {
    module = LLVMModuleCreateWithName("module");
    builder = LLVMCreateBuilder();

    {
        MapIterator iter = map_iter(ir->subs);
        IRSub *sub;
        while (map_iter_next(&iter, NULL, (void **)&sub))
            codegen_sub(sub);
    }

    return module;
}
