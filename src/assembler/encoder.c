#include <stdlib.h>
#include <assert.h>

#include "encoder.h"

#define INDIRECT_MASK (1u << 15)

void encode_instruction(EncoderState *state, const Statement *stmt);

void encode_mr_instruction(EncoderState *state, const Statement *stmt);
void encode_nmr_instruction(EncoderState *state, const Statement *stmt);
void encode_ps_instruction(EncoderState *state, const Statement *stmt);

EncoderState encoder_new(Vector statements) {
    uint16_t *buffer = calloc(MANO_MEMORY_SIZE, sizeof(uint16_t));

    assert(buffer != NULL && "Failed to allocate Mano memory");

    return (EncoderState) {
        .statements = statements,
        .memory = buffer,
    };
}

void encode_instructions(EncoderState *state) {
    size_t len = state->statements.size;

    for (size_t i = 0; i < len; i++) {
        const Statement *stmt = vector_at_const(&state->statements, i);
        encode_instruction(state, stmt);
    }
}

void encode_instruction(EncoderState *state, const Statement *stmt) {
    Instruction inst = stmt->instruction;
    switch (inst) {
        case INST_PSI_ORG:
        case INST_PSI_DEC:
        case INST_PSI_HEX:
        case INST_PSI_END:
            encode_ps_instruction(state, stmt);
            return;
        default:
            break;
    }

    if (inst >= 0 && inst <= 6) {
        encode_mr_instruction(state, stmt);
    } else if (inst >> 12 == 0x7 || inst >> 12 == 0xF) {
        encode_nmr_instruction(state, stmt);
    } else {
        assert(false && "Unknown instruction");
    }
}

void encode_mr_instruction(EncoderState *state, const Statement *stmt) {
    uint16_t inst = stmt->instruction << 12;
    if (stmt->indirect) {
        inst |= INDIRECT_MASK;
    }
    inst |= stmt->operand;

    state->memory[stmt->location] = inst;
}

void encode_nmr_instruction(EncoderState *state, const Statement *stmt) {
    state->memory[stmt->location] = stmt->instruction;
}

void encode_ps_instruction(EncoderState *state, const Statement *stmt) {
    switch (stmt->instruction) {
        case INST_PSI_ORG:
        case INST_PSI_END:
            break;
        case INST_PSI_DEC:
        case INST_PSI_HEX:
            state->memory[stmt->location] = stmt->operand;
            break;
        default:
            assert(false && "Unreachable!");
    }
}