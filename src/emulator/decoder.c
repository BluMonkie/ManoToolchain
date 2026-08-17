#include <stdio.h>
#include <stdlib.h>

#include "decoder.h"

#define MR_OPCODE_MASK   0x7000
#define INDIRECT_MASK 0x8000
#define MR_ADDRESS_MASK  0x0FFF

static Instruction decode_mr_instruction(Word word);
static Instruction decode_rr_instruction(Word word);
static Instruction decode_io_instruction(Word word);

Instruction decode_instruction(Word word) {
    uint16_t opcode = word >> 12;

    if (opcode == 0x7) {
        return decode_rr_instruction(word);
    } else if (opcode == 0xF) {
        return decode_io_instruction(word);
    }

    return decode_mr_instruction(word);
}

Instruction decode_mr_instruction(Word word) {
    uint16_t opcode = (word & MR_OPCODE_MASK) >> 12;
    bool is_indirect = (word & INDIRECT_MASK) != 0;
    Address operand = (word & MR_ADDRESS_MASK);

    InstOpcode inst = (InstOpcode) opcode;
    return (Instruction) {
        .opcode = inst, 
        .operand = operand,
        .is_indirect = is_indirect,
    };
}

Instruction decode_rr_instruction(Word word) {
    InstOpcode inst = INST_INVALID;

    switch (word) {
        case 0x7800: inst = INST_CLA; break;
        case 0x7400: inst = INST_CLE; break;
        case 0x7200: inst = INST_CMA; break;
        case 0x7100: inst = INST_CME; break;
        case 0x7080: inst = INST_CIR; break;
        case 0x7040: inst = INST_CIL; break;
        case 0x7020: inst = INST_INC; break;
        case 0x7010: inst = INST_SPA; break;
        case 0x7008: inst = INST_SNA; break;
        case 0x7004: inst = INST_SZA; break;
        case 0x7002: inst = INST_SZE; break;
        case 0x7001: inst = INST_HLT; break;
    }

    return (Instruction) {
        .opcode = inst,
        .operand = INVALID_ADDRESS
    };
}

Instruction decode_io_instruction(Word word) {
    InstOpcode inst = INST_INVALID;

    switch (word) {
        case 0xF800: inst = INST_INP; break;
        case 0xF400: inst = INST_OUT; break;
        case 0xF200: inst = INST_SKI; break;
        case 0xF100: inst = INST_SKO; break;
        case 0xF080: inst = INST_ION; break;
        case 0xF040: inst = INST_IOF; break;
    }

    return (Instruction) {
        .opcode = inst,
        .operand = INVALID_ADDRESS
    };
}