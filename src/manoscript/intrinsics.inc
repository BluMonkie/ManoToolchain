#include "defs.h"
#include "intrinsics.h"
#include "codegen.h"
#include "symbol_map.h"
#include <stddef.h>

static void emit(CodegenState *state, ManoInst inst);
static const char *get_intrinsic_arg(CodegenState *state, Intrinsic intr);

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

static void emit_bit_lsh(CodegenState *state, const char *label) {
    const char *arg = get_intrinsic_arg(state, INT_BIT_LSH);
    const char *temp = symbol_get_label(&state->symbols);
    const char *llsh = symbol_get_label(&state->symbols);
    const char *lloop = symbol_get_label(&state->symbols);

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
        .argument = llsh,
    });
    emit(state, (ManoInst) {
        .type = INST_MEM_LDA,
        .argument = arg,
    });
    emit(state, (ManoInst) {
        .type = INST_MEM_BUN,
        .argument = label,
        .indirect = true,
    });
    emit(state, (ManoInst) {
        .type = INST_REG_CMA,
        .label = llsh,
    });
    emit(state, (ManoInst) {
        .type = INST_REG_INC,
    });
    emit(state, (ManoInst) {
        .type = INST_MEM_STA,
        .argument = temp,
    });
    emit(state, (ManoInst) {
        .type = INST_MEM_LDA,
        .argument = arg,
    });
    emit(state, (ManoInst) {
        .type = INST_REG_CLE,
        .label = lloop,
    });
    emit(state, (ManoInst) {
        .type = INST_REG_CIL,
    });
    emit(state, (ManoInst) {
        .type = INST_MEM_ISZ,
        .argument = temp,
    });
    emit(state, (ManoInst) {
        .type = INST_MEM_BUN,
        .argument = lloop,
    });
    emit(state, (ManoInst) {
        .type = INST_MEM_BUN,
        .argument = label,
        .indirect = true,
    });
    emit(state, (ManoInst) {
        .type = INST_PSI_HEX,
        .argument = "0",
        .label = arg,
    });
    emit(state, (ManoInst) {
        .type = INST_PSI_HEX,
        .argument = "0",
        .label = temp,
    });
}

static void emit_bit_rsh(CodegenState *state, const char *label) {
    const char *arg = get_intrinsic_arg(state, INT_BIT_RSH);
    const char *temp = symbol_get_label(&state->symbols);
    const char *lrsh = symbol_get_label(&state->symbols);
    const char *lloop = symbol_get_label(&state->symbols);

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
        .argument = lrsh,
    });
    emit(state, (ManoInst) {
        .type = INST_MEM_LDA,
        .argument = arg,
    });
    emit(state, (ManoInst) {
        .type = INST_MEM_BUN,
        .argument = label,
        .indirect = true,
    });
    emit(state, (ManoInst) {
        .type = INST_REG_CMA,
        .label = lrsh,
    });
    emit(state, (ManoInst) {
        .type = INST_REG_INC,
    });
    emit(state, (ManoInst) {
        .type = INST_MEM_STA,
        .argument = temp,
    });
    emit(state, (ManoInst) {
        .type = INST_MEM_LDA,
        .argument = arg,
    });
    emit(state, (ManoInst) {
        .type = INST_REG_CLE,
        .label = lloop,
    });
    emit(state, (ManoInst) {
        .type = INST_REG_CIR,
    });
    emit(state, (ManoInst) {
        .type = INST_MEM_ISZ,
        .argument = temp,
    });
    emit(state, (ManoInst) {
        .type = INST_MEM_BUN,
        .argument = lloop,
    });
    emit(state, (ManoInst) {
        .type = INST_MEM_BUN,
        .argument = label,
        .indirect = true,
    });
    emit(state, (ManoInst) {
        .type = INST_PSI_HEX,
        .argument = "0",
        .label = arg,
    });
    emit(state, (ManoInst) {
        .type = INST_PSI_HEX,
        .argument = "0",
        .label = temp,
    });
}

const IntrinsicEmitter emitters[] = {
    [INT_CMP_LS] = emit_cmp_ls,
    [INT_CMP_GR] = emit_cmp_gr,
    [INT_CMP_LS_EQ] = emit_cmp_lseq,
    [INT_CMP_GR_EQ] = emit_cmp_greq,
    [INT_CMP_EQ] = emit_cmp_eq,
    [INT_CMP_NEQ] = emit_cmp_neq,
    [INT_BIT_LSH] = emit_bit_lsh,
    [INT_BIT_RSH] = emit_bit_rsh,
};

static void emit_intrinsics(CodegenState *state) {
    for (size_t i = 0; i < INT_COUNT; i++) {
        IntrinsicEntry *entry = &state->intrinsics[i];
        
        if (!entry->enabled) continue;
        emitters[entry->intr](state, entry->label);
    }
}

static const char *get_intrinsic_arg(CodegenState *state, Intrinsic intr) {
    const char **label = &state->intrinsics[intr].arg_label;
    if (*label == nullptr) {
        *label = symbol_get_label(&state->symbols);
    }
    return *label;
}