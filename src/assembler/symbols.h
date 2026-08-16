#pragma once

#include "defs.h"
#include "common/vector.h"

typedef struct {
    Token identifier;
    Address location;
} SymbolLocation;

typedef struct {
    char *src;
    Vector symbols;
    bool had_error;
} SymbolTable;

SymbolTable symbol_table_new(char *src);

bool is_symbol_defined(SymbolTable *table, Token identifier);

void symbol_define(SymbolTable *table, Token identifier, Address location);

const SymbolLocation *symbol_find(SymbolTable *table, Token identifier);

Address symbol_locate(SymbolTable *table, Token identifier);