#pragma once

#include "ast.h"
#include "common/vector.h"
#include "symbol_map.h"

typedef struct {
    Vector instructions;
    Program *program;
    SymbolMap symbols;
    Vector temporaries;
    int temp_depth;
} CodegenState;

CodegenState codegen_new(Program *program);

void emit_instructions(CodegenState *state);