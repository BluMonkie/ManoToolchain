#pragma once

#include "common/vector.h"
#include "ast.h"
#include "common/arena.h"

typedef struct {
    char *src;
    bool had_error;
    size_t current;
    Vector tokens;
    Arena arena;
} ParserState;

ParserState parser_new(char *src, Vector tokens);
Program parse_program(ParserState *state);