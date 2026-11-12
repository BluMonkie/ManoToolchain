#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "ast.h"
#include "common/arena.h"
#include "common/vector.h"
#include "defs.h"

typedef struct {
    StringSlice var_name;
    char *mangled;
} VarPair;

typedef struct {
    StringSlice var_name;
    size_t index;
} VarIndexPair;

typedef struct {
    int value;
    char *value_str;
    char *mangled;
} ConstPair;

typedef struct {
    char *label;
    Stmt *stmt;
    char *return_label;
} LabelPair;

typedef struct Scope {
    struct Scope *parent;
    Vector vars;
} Scope;

typedef struct {
    Scope *scope;
    Vector var_map;
    Vector const_map;
    Vector label_map;
    Arena arena;
} SymbolMap;

SymbolMap symbol_map_new();
void symbol_map_free(SymbolMap* map);

bool symbol_define_var(SymbolMap *map, StringSlice str);
bool symbol_exists(SymbolMap *map, StringSlice str);
bool symbol_exists_in_scope(SymbolMap *map, StringSlice str);
const char *symbol_get_map_var(SymbolMap *map, StringSlice str);
const char *symbol_get_map_const(SymbolMap *map, int num);
const char *symbol_get_label(SymbolMap *map);
const char *symbol_get_label_for_block(SymbolMap *map, Stmt *stmt, char *return_label);

void symbol_push_scope(SymbolMap *map);
void symbol_pop_scope(SymbolMap *map);