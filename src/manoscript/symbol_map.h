#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "ast.h"
#include "common/arena.h"
#include "common/vector.h"
#include "defs.h"

typedef enum {
    SYMBOL_VAR,
    SYMBOL_FUN,
} SymbolType;

typedef struct {
    StringSlice sym_name;
    char *mangled;
    SymbolType type;
} SymbolPair;

typedef struct {
    StringSlice sym_name;
    size_t index;
} SymbolIndexPair;

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
    Vector symbols;
} Scope;

typedef struct {
    FunDeclStmt *fun;
    Scope *scope;
} FunScopePair;

typedef struct {
    Scope *scope;
    Vector symbol_map;
    Vector const_map;
    Vector label_map;
    Vector functions;
    Vector fun_scopes;
    Arena arena;
} SymbolMap;

SymbolMap symbol_map_new();
void symbol_map_free(SymbolMap* map);

bool symbol_define_symbol(SymbolMap *map, StringSlice str);
bool symbol_define_function(SymbolMap *map, FunDeclStmt *fun);
bool symbol_exists(SymbolMap *map, StringSlice str);
bool symbol_exists_in_scope(SymbolMap *map, StringSlice str);
const char *symbol_get_map_symbol(SymbolMap *map, StringSlice str);
const char *symbol_get_map_const(SymbolMap *map, int num);
FunDeclStmt *symbol_get_function(SymbolMap *map, StringSlice str);
const char *symbol_get_label(SymbolMap *map);
const char *symbol_get_label_for_block(SymbolMap *map, Stmt *stmt, const char *return_label);

void symbol_push_scope(SymbolMap *map);
void symbol_pop_scope(SymbolMap *map);
void restore_fun_scope(SymbolMap *map, FunDeclStmt *fun);