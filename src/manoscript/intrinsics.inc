#include "defs.h"
#include "intrinsics.h"
#include "codegen.h"
#include "symbol_map.h"
#include <stddef.h>

static const char *get_next_temporary(CodegenState *state);
static const char *pop_temporary(CodegenState *state);
static void emit(CodegenState *state, ManoInst inst);

typedef void (*IntrinsicEmitter)(CodegenState *state, const char*);

const char *enable_intrinsic(CodegenState *state, Intrinsic intr) {
    IntrinsicEntry *entry = &state->intrinsics[intr];
    if (entry->enabled) {
        return entry->label;
    }

    entry->label = symbol_get_label(&state->symbols);
    entry->enabled = true;
    entry->intr = intr;

    return entry->label;
}

static void emit_cmp_ls(CodegenState *state, const char *label) {
    const char *true_c = symbol_get_map_const(&state->symbols, 1);

    const char *false_lab = symbol_get_label(&state->symbols);

    emit(state, (ManoInst) {
        .type = INST_PSI_HEX,
        .argument = "0",
        .label = label,
    });
    emit(state, (ManoInst) {
        .type = INST_REG_SNA,
    }); 
    emit(state, (ManoInst) {
        .type = INST_MEM_BUN,
        .argument = false_lab,
    }); 
    emit(state, (ManoInst) {
        .type = INST_MEM_LDA,
        .argument = true_c,
    }); 
    emit(state, (ManoInst) {
        .type = INST_MEM_BUN,
        .argument = label,
        .indirect = true,
    }); 
    emit(state, (ManoInst) {
        .type = INST_REG_CLA,
        .label = false_lab,
    }); 
    emit(state, (ManoInst) {
        .type = INST_MEM_BUN,
        .argument = label,
        .indirect = true,
    }); 
}

static void emit_cmp_gr(CodegenState *state, const char *label) {
    const char *true_c = symbol_get_map_const(&state->symbols, 1);

    const char *false_lab = symbol_get_label(&state->symbols);

    emit(state, (ManoInst) {
        .type = INST_PSI_HEX,
        .argument = "0",
        .label = label,
    });
    emit(state, (ManoInst) {
        .type = INST_REG_SPA,
    }); 
    emit(state, (ManoInst) {
        .type = INST_MEM_BUN,
        .argument = false_lab,
    }); 
    emit(state, (ManoInst) {
        .type = INST_MEM_LDA,
        .argument = true_c,
    }); 
    emit(state, (ManoInst) {
        .type = INST_MEM_BUN,
        .argument = label,
        .indirect = true,
    }); 
    emit(state, (ManoInst) {
        .type = INST_REG_CLA,
        .label = false_lab,
    }); 
    emit(state, (ManoInst) {
        .type = INST_MEM_BUN,
        .argument = label,
        .indirect = true,
    }); 
}

static void emit_cmp_lseq(CodegenState *state, const char *label) {
    const char *true_c = symbol_get_map_const(&state->symbols, 1);

    const char *true_lab = symbol_get_label(&state->symbols);

    emit(state, (ManoInst) {
        .type = INST_PSI_HEX,
        .argument = "0",
        .label = label,
    });
    emit(state, (ManoInst) {
        .type = INST_REG_SPA,
    }); 
    emit(state, (ManoInst) {
        .type = INST_MEM_BUN,
        .argument = true_lab,
    }); 
    emit(state, (ManoInst) {
        .type = INST_REG_CLA,
    }); 
    emit(state, (ManoInst) {
        .type = INST_MEM_BUN,
        .argument = label,
        .indirect = true,
    }); 
    emit(state, (ManoInst) {
        .type = INST_MEM_LDA,
        .argument = true_c,
        .label = true_lab,
    }); 
    emit(state, (ManoInst) {
        .type = INST_MEM_BUN,
        .argument = label,
        .indirect = true,
    }); 
}

static void emit_cmp_greq(CodegenState *state, const char *label) {
    const char *true_c = symbol_get_map_const(&state->symbols, 1);

    const char *true_lab = symbol_get_label(&state->symbols);

    emit(state, (ManoInst) {
        .type = INST_PSI_HEX,
        .argument = "0",
        .label = label,
    });
    emit(state, (ManoInst) {
        .type = INST_REG_SNA,
    }); 
    emit(state, (ManoInst) {
        .type = INST_MEM_BUN,
        .argument = true_lab,
    }); 
    emit(state, (ManoInst) {
        .type = INST_REG_CLA,
    }); 
    emit(state, (ManoInst) {
        .type = INST_MEM_BUN,
        .argument = label,
        .indirect = true,
    }); 
    emit(state, (ManoInst) {
        .type = INST_MEM_LDA,
        .argument = true_c,
        .label = true_lab,
    }); 
    emit(state, (ManoInst) {
        .type = INST_MEM_BUN,
        .argument = label,
        .indirect = true,
    }); 
}

static void emit_cmp_eq(CodegenState *state, const char *label) {
    const char *true_c = symbol_get_map_const(&state->symbols, 1);

    const char *false_lab = symbol_get_label(&state->symbols);

    emit(state, (ManoInst) {
        .type = INST_PSI_HEX,
        .argument = "0",
        .label = label,
    });
    emit(state, (ManoInst) {
        .type = INST_REG_SZA,
    }); 
    emit(state, (ManoInst) {
        .type = INST_MEM_BUN,
        .argument = false_lab,
    }); 
    emit(state, (ManoInst) {
        .type = INST_MEM_LDA,
        .argument = true_c,
    }); 
    emit(state, (ManoInst) {
        .type = INST_MEM_BUN,
        .argument = label,
        .indirect = true,
    }); 
    emit(state, (ManoInst) {
        .type = INST_REG_CLA,
        .label = false_lab,
    }); 
    emit(state, (ManoInst) {
        .type = INST_MEM_BUN,
        .argument = label,
        .indirect = true,
    }); 
}

static void emit_cmp_neq(CodegenState *state, const char *label) {
    const char *true_c = symbol_get_map_const(&state->symbols, 1);

    const char *true_lab = symbol_get_label(&state->symbols);

    emit(state, (ManoInst) {
        .type = INST_PSI_HEX,
        .argument = "0",
        .label = label,
    });
    emit(state, (ManoInst) {
        .type = INST_REG_SZA,
    }); 
    emit(state, (ManoInst) {
        .type = INST_MEM_BUN,
        .argument = true_lab,
    }); 
    emit(state, (ManoInst) {
        .type = INST_REG_CLA,
    }); 
    emit(state, (ManoInst) {
        .type = INST_MEM_BUN,
        .argument = label,
        .indirect = true,
    }); 
    emit(state, (ManoInst) {
        .type = INST_MEM_LDA,
        .argument = true_c,
        .label = true_lab,
    }); 
    emit(state, (ManoInst) {
        .type = INST_MEM_BUN,
        .argument = label,
        .indirect = true,
    }); 
}

const IntrinsicEmitter emitters[] = {
    [INT_CMP_LS] = emit_cmp_ls,
    [INT_CMP_GR] = emit_cmp_gr,
    [INT_CMP_LS_EQ] = emit_cmp_lseq,
    [INT_CMP_GR_EQ] = emit_cmp_greq,
    [INT_CMP_EQ] = emit_cmp_eq,
    [INT_CMP_NEQ] = emit_cmp_neq,
};

static void emit_intrinsics(CodegenState *state) {
    for (size_t i = 0; i < INT_COUNT; i++) {
        IntrinsicEntry *entry = &state->intrinsics[i];
        
        if (!entry->enabled) continue;
        emitters[entry->intr](state, entry->label);
    }
}