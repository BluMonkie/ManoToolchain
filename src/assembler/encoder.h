#pragma once

#include <stdint.h>

#include "defs.h"
#include "common/vector.h"

#define MANO_MEMORY_SIZE 4096

typedef struct {
    Vector statements;
    uint16_t *memory;
} EncoderState;

EncoderState encoder_new(Vector statements); 
void encode_instructions(EncoderState *state);