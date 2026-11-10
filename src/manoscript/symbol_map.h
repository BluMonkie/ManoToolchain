#pragma once

#include <stdbool.h>

#include "common/vector.h"
#include "defs.h"

typedef struct {
    StringSlice var_name;
    char mangled[4];
} VarPair;

typedef struct {
    int value;
    char mangled[4];
} ConstPair;

typedef struct {
    Vector var_map;
    Vector const_map;
} SymbolMap;

SymbolMap symbol_map_new();
void symbol_map_free(SymbolMap* map);

bool symbol_define_var(SymbolMap *map, StringSlice str);
bool symbol_exists(SymbolMap *map, StringSlice str);
const char *symbol_get_map_var(SymbolMap *map, StringSlice str);
const char *symbol_get_map_const(SymbolMap *map, int num);