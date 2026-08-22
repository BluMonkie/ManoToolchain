#pragma once

#include "defs.h"
#include "common/vector.h"

typedef struct {
    const char *src;
    size_t begin;
    size_t current;
    Vector tokens;
    bool had_error;
} LexerState;

LexerState lexer_new(const char *src);
void scan_tokens(LexerState *state);