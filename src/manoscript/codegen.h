#pragma once

#include "common/arena.h"
#include "ast.h"
#include "common/vector.h"
#include "symbol_map.h"
#include "intrinsics.h"

typedef struct {
    Vector instructions;
    Program *program;
    SymbolMap symbols;
    Vector temporaries;
    Arena arena;
    IntrinsicEntry intrinsics[INT_COUNT];
    int temp_depth;
    const char *pending_label;
} CodegenState;

CodegenState codegen_new(Program *program);
void codegen_free(CodegenState *state);

void emit_instructions(CodegenState *state);