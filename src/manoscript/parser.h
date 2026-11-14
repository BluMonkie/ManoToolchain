#pragma once

#include "common/vector.h"
#include "ast.h"
#include "common/arena.h"

typedef struct {
    const char *src;
    bool had_error;
    size_t current;
    Vector tokens;
    Arena arena;
    Vector statements;
} ParserState;

ParserState parser_new();
Program parse_program(ParserState *state, const char *src, Vector tokens);
Program parse_program_builtin(ParserState *state, const char *src, Vector tokens);