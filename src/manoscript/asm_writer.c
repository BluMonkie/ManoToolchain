#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "asm_writer.h"
#include "defs.h"
#include "common/vector.h"

static const char *get_inst_name(ManoInstType type);
static void write_inst(const ManoInst *inst, FILE *stream);

void write_asm(Vector instructions, FILE *stream) {
    for (size_t i = 0; i < instructions.size; i++) {
        ManoInst *inst = vector_at(&instructions, i);
        write_inst(inst, stream);
    }
}

static void write_inst(const ManoInst *inst, FILE *stream) {
    ManoInstType type = inst->type;

    if (inst->label != nullptr) {
        fprintf(stream, "%s, ", inst->label);
    }

    if (type >= INST_MEM_AND && type <= INST_MEM_ISZ) {
        fprintf(stream, "%s %s", get_inst_name(type), inst->argument);
        if (inst->indirect) {
            fprintf(stream, " I");
        }
    } else if ((type >= INST_REG_CLA && type <= INST_IO_IOF) || type == INST_PSI_END) {
        fprintf(stream, "%s", get_inst_name(type));
    } else if (type == INST_ASM) {
        fprintf(stream, "%s", inst->argument);
    } else {
        fprintf(stream, "%s %s", get_inst_name(type), inst->argument);
    }
    fprintf(stream, "\n");
}

static const char *get_inst_name(ManoInstType type) {
    static const char *names[] = {
        // Memory Reference
        [INST_MEM_AND] = "AND",
        [INST_MEM_ADD] = "ADD",
        [INST_MEM_LDA] = "LDA",
        [INST_MEM_STA] = "STA",
        [INST_MEM_BUN] = "BUN",
        [INST_MEM_BSA] = "BSA",
        [INST_MEM_ISZ] = "ISZ",

        // Register Reference
        [INST_REG_CLA] = "CLA",
        [INST_REG_CLE] = "CLE",
        [INST_REG_CMA] = "CMA",
        [INST_REG_CME] = "CME",
        [INST_REG_CIR] = "CIR",
        [INST_REG_CIL] = "CIL",
        [INST_REG_INC] = "INC",
        [INST_REG_SPA] = "SPA",
        [INST_REG_SNA] = "SNA",
        [INST_REG_SZA] = "SZA",
        [INST_REG_SZE] = "SZE",
        [INST_REG_HLT] = "HLT",

        // I/O Instructions
        [INST_IO_INP]  = "INP",
        [INST_IO_OUT]  = "OUT",
        [INST_IO_SKI]  = "SKI",
        [INST_IO_SKO]  = "SKO",
        [INST_IO_ION]  = "ION",
        [INST_IO_IOF]  = "IOF",

        // Pseudo-Instructions
        [INST_PSI_ORG] = "ORG",
        [INST_PSI_DEC] = "DEC",
        [INST_PSI_HEX] = "HEX",
        [INST_PSI_END] = "END",
    };

    if ((size_t)type >= sizeof(names) / sizeof(names[0]) || names[type] == NULL) {
        return "UNKNOWN";
    }

    return names[type];
}