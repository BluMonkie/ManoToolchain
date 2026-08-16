#pragma once

#include <defs.h>
#include <stdint.h>
#include <stdbool.h>
#include <common/vector.h>

typedef struct {
    char *src;
    size_t begin;
    size_t current;
    Vector tokens;
    bool had_error;
} LexerState;

LexerState lexer_new(char *src);

void lexer_scan_tokens(LexerState *state);