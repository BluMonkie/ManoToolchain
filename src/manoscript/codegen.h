#pragma once

#include "common/arena.h"
#include "ast.h"
#include "common/vector.h"
#include "symbol_map.h"

typedef struct {
    Vector instructions;
    Program *program;
    SymbolMap symbols;
    Vector temporaries;
    Arena arena;
    int temp_depth;
    char *pending_label;
} CodegenState;

CodegenState codegen_new(Program *program);
void codegen_free(CodegenState *state);

void emit_instructions(CodegenState *state);