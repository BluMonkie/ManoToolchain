#pragma once

#include <lexer.h>
#include <parser.h>

const char *token_type_name(TokenType type);
const char *instruction_name(Instruction instruction);
void print_tokens(const LexerState *state);
void print_symbols(const SymbolTable *table);
void print_ast(const ParserState *state);