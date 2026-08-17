#pragma once

#include "defs.h"
#include "common/vector.h"

typedef struct {
    const uint16_t *buffer;
    const size_t buffer_len;
    Vector instructions;
} DecoderState;

DecoderState decoder_new(const Word *buffer, size_t buffer_len);
void decode_instructions(DecoderState *state);