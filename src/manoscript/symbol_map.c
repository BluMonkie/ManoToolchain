#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "symbol_map.h"
#include "ast.h"
#include "common/arena.h"
#include "common/vector.h"
#include "defs.h"

static void next_var_sym(SymbolMap *map, char out[4]);
static void next_const_sym(SymbolMap *map, char out[4]);
static void next_label_sym(SymbolMap *map, char out[4]);
static void next_fun_label_sym(SymbolMap *map, char out[4]);
static char get_base36_char(unsigned int i);

static Scope *scope_new(SymbolMap *map, Scope *parent);
static void scope_free(Scope *scope);

SymbolMap symbol_map_new() {
    SymbolMap map = {
        .symbol_map = vector_new(sizeof(SymbolPair)),
        .const_map = vector_new(sizeof(ConstPair)),
        .label_map = vector_new(sizeof(LabelPair)),
        .functions = vector_new(sizeof(FunDeclStmt)),
        .fun_scopes = vector_new(sizeof(FunScopePair)),
        .arena = arena_new(),
    };

    map.scope = scope_new(&map, nullptr);
    return map;
}

void symbol_map_free(SymbolMap *map) {
    scope_free(map->scope);
    vector_free(&map->symbol_map);
    vector_free(&map->const_map);
    vector_free(&map->label_map);
    vector_free(&map->functions);
    vector_free(&map->fun_scopes);
    arena_drop(&map->arena);
}

bool symbol_define_symbol(SymbolMap *map, StringSlice str) {
    if (symbol_exists_in_scope(map, str)) return false;

    char *mangled = arena_alloc(&map->arena, sizeof(char) * 4);
    next_var_sym(map, mangled);
    
    SymbolPair pair = {
        .sym_name = str,
        .mangled = mangled,
        .type = SYMBOL_VAR,
    };

    vector_push(&map->symbol_map, &pair);

    SymbolIndexPair sid = {
        .sym_name = str,
        .index = map->symbol_map.size - 1,
    };

    vector_push(&map->scope->symbols, &sid);

    return true;
}

bool symbol_define_function(SymbolMap *map, FunDeclStmt *fun) {
    if (symbol_exists(map, fun->identifier->lexeme)) {
        return false;
    }

    char *mangled = arena_alloc(&map->arena, sizeof(char) * 4);
    next_fun_label_sym(map, mangled);
    
    SymbolPair pair = {
        .sym_name = fun->identifier->lexeme,
        .mangled = mangled,
        .type = SYMBOL_FUN,
    };
    vector_push(&map->symbol_map, &pair);

    SymbolIndexPair sid = {
        .sym_name = fun->identifier->lexeme,
        .index = map->symbol_map.size - 1,
    };

    vector_push(&map->scope->symbols, &sid);

    symbol_push_scope(map);
    for (size_t i = 0; i < fun->parameters.size; i++) {
        FunParameter *param = vector_at(&fun->parameters, i);
        symbol_define_symbol(map, param->identifier->lexeme);
        param->mangled = symbol_get_map_symbol(map, param->identifier->lexeme);
    }
    if (fun->output != nullptr) {
        FunParameter *param = fun->output;
        symbol_define_symbol(map, param->identifier->lexeme);
        param->mangled = symbol_get_map_symbol(map, param->identifier->lexeme);
    }
    FunScopePair fsp = {
        .fun = fun,
        .scope = map->scope,
    };
    map->scope = map->scope->parent;

    vector_push(&map->functions, fun);
    vector_push(&map->fun_scopes, &fsp);
    return true;
}

bool symbol_exists(SymbolMap *map, StringSlice str) {
    return symbol_get_map_symbol(map, str) != nullptr;
}

bool symbol_exists_in_scope(SymbolMap *map, StringSlice str) {
    Scope *scope = map->scope;

    for (size_t i = 0; i < scope->symbols.size; i++) {
        SymbolIndexPair *vid = vector_at(&scope->symbols, i);
        StringSlice *name = &vid->sym_name;

        if (str.len == name->len && memcmp(str.start, name->start, str.len) == 0) {
            return true;
        }
    }

    return false;
}

const char *symbol_get_map_symbol(SymbolMap *map, StringSlice str) {
    Scope *scope = map->scope;

    while (scope != nullptr) {
        for (size_t i = 0; i < scope->symbols.size; i++) {
            SymbolIndexPair *vid = vector_at(&scope->symbols, i);
            StringSlice *name = &vid->sym_name;

            if (str.len == name->len && memcmp(str.start, name->start, str.len) == 0) {
                return ((SymbolPair *) vector_at(&map->symbol_map, vid->index))->mangled;
            }
        }
        scope = scope->parent;
    }

    return nullptr;
}

FunDeclStmt *symbol_get_function(SymbolMap *map, StringSlice str) {
    for (size_t i = 0; i < map->functions.size; i++) {
        FunDeclStmt *fun = vector_at(&map->functions, i);
        StringSlice *name = &fun->identifier->lexeme;

        if (str.len == name->len && memcmp(str.start, name->start, str.len) == 0) {
            return fun;
        }
    }
    return nullptr;
}

const char *symbol_get_map_const(SymbolMap *map, int num) {
    for (size_t i = 0; i < map->const_map.size; i++) {
        ConstPair *pair = vector_at(&map->const_map, i);

        if (pair->value == num) {
            return pair->mangled;
        }
    }

    char *mangled = arena_alloc(&map->arena, sizeof(char) * 4);
    next_const_sym(map, mangled);

    char *value_str = arena_alloc(&map->arena, 16);
    snprintf(value_str, 16, "%d", num);

    ConstPair pair = {
        .value = num,
        .value_str = value_str,
        .mangled = mangled,
    };

    vector_push(&map->const_map, &pair);

    return mangled;
}

const char *symbol_get_label(SymbolMap *map) {
    char *label = arena_alloc(&map->arena, sizeof(char) * 4);
    next_label_sym(map, label);

    LabelPair pair = {
        .label = label,
        .stmt = nullptr,
        .return_label = nullptr,
    };

    vector_push(&map->label_map, &pair);
    return label;
}

const char *symbol_get_label_for_block(SymbolMap *map, Stmt *stmt, const char *return_label) {
    char *label = arena_alloc(&map->arena, sizeof(char) * 4);
    next_label_sym(map, label);

    LabelPair pair = {
        .label = label,
        .stmt = stmt,
        .return_label = arena_alloc_type(&map->arena, sizeof(char) * 4, return_label),
    };

    vector_push(&map->label_map, &pair);

    return label;
}

void symbol_push_scope(SymbolMap *map) {
    Scope *scope = scope_new(map, map->scope);

    map->scope = scope;
}

void symbol_pop_scope(SymbolMap *map) {
    Scope *scope = map->scope;
    map->scope = scope->parent;

    scope_free(scope);
}

void restore_fun_scope(SymbolMap *map, FunDeclStmt *fun) {
    StringSlice str = fun->identifier->lexeme;
    Scope *scope;

    for (size_t i = 0; i < map->fun_scopes.size; i++) {
        FunScopePair *fsp = vector_at(&map->fun_scopes, i);
        StringSlice *name = &fsp->fun->identifier->lexeme;

        if (str.len == name->len && memcmp(str.start, name->start, str.len) == 0) {
            scope = fsp->scope;
        }
    }

    scope->parent = map->scope;
    map->scope = scope;
}

static void next_var_sym(SymbolMap *map, char out[4]) {
    int size = map->symbol_map.size;

    out[0] = 'w' + (size / (36 * 36));
    out[1] = get_base36_char(size / 36);
    out[2] = get_base36_char(size % 36);
    out[3] = '\0';
}

static void next_const_sym(SymbolMap *map, char out[4]) {
    int size = map->const_map.size;

    out[0] = 'a' + (size / (36 * 36));
    out[1] = get_base36_char(size / 36);
    out[2] = get_base36_char(size % 36);
    out[3] = '\0';
}

static void next_label_sym(SymbolMap *map, char out[4]) {
    int size = map->label_map.size;
    
    out[0] = 'l' + (size / (36 * 36));
    out[1] = get_base36_char(size / 36);
    out[2] = get_base36_char(size % 36);
    out[3] = '\0';
}

static void next_fun_label_sym(SymbolMap *map, char out[4]) {
    int size = map->functions.size;
    
    out[0] = 'e' + (size / (36 * 36));
    out[1] = get_base36_char(size / 36);
    out[2] = get_base36_char(size % 36);
    out[3] = '\0';
}

static char get_base36_char(unsigned int i) {
    static const char alphanum[] = "0123456789abcdefghijklmnopqrstuvwxyz";
    
    return alphanum[i % 36];
}

static Scope *scope_new(SymbolMap *map, Scope *parent) {
    Scope *scope = arena_alloc(&map->arena, sizeof(Scope));
    scope->parent = parent;
    scope->symbols = vector_new(sizeof(SymbolIndexPair));

    return scope;
}

static void scope_free(Scope *scope) {
    vector_free(&scope->symbols);
}