#pragma once

#include <stdio.h>

#include <lexer.h>
#include <parser.h>

const char *token_type_name(TokenType type);
const char *instruction_name(Instruction instruction);
void print_tokens(const LexerState *state, FILE *stream);
void print_symbols(const SymbolTable *table, FILE *stream);
void print_ast(const ParserState *state, FILE *stream);