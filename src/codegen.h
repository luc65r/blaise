#pragma once

#include <llvm-c/Core.h>

#include "ir.h"

LLVMModuleRef codegen(IR *ir);
