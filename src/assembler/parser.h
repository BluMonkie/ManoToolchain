#pragma once

#include "defs.h"
#include "common/vector.h"
#include "symbols.h"

typedef struct {
    char *src;
    bool had_error;
    size_t current;
    Address current_address;
    Vector tokens;
    Vector statements;
    SymbolTable symbols;
} ParserState;

ParserState parser_new(char *src, Vector tokens);
void parse_tokens(ParserState *state);