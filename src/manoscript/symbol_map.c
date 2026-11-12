#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "symbol_map.h"
#include "common/arena.h"
#include "common/vector.h"
#include <stdio.h>
#include "common/xmalloc.h"
#include "defs.h"

static void next_var_sym(SymbolMap *map, char out[4]);
static void next_const_sym(SymbolMap *map, char out[4]);
static char get_base36_char(unsigned int i);

static Scope *scope_new(Scope *parent);
static void scope_free(Scope *scope);

SymbolMap symbol_map_new() {
    return (SymbolMap) {
        .scope = scope_new(nullptr),
        .var_map = vector_new(sizeof(VarPair)),
        .const_map = vector_new(sizeof(ConstPair)),
        .arena = arena_new(),
    };
}

void symbol_map_free(SymbolMap *map) {
    scope_free(map->scope);
    vector_free(&map->var_map);
    vector_free(&map->const_map);
    arena_drop(&map->arena);
}

bool symbol_define_var(SymbolMap *map, StringSlice str) {
    if (symbol_exists_in_scope(map, str)) return false;

    char *mangled = arena_alloc(&map->arena, sizeof(char) * 4);
    next_var_sym(map, mangled);
    
    VarPair pair = {
        .var_name = str,
        .mangled = mangled,
    };

    vector_push(&map->var_map, &pair);

    VarIndexPair vid = {
        .var_name = str,
        .index = map->var_map.size - 1,
    };

    vector_push(&map->scope->vars, &vid);

    return true;
}

bool symbol_exists(SymbolMap *map, StringSlice str) {
    return symbol_get_map_var(map, str) != nullptr;
}

bool symbol_exists_in_scope(SymbolMap *map, StringSlice str) {
    Scope *scope = map->scope;

    for (size_t i = 0; i < scope->vars.size; i++) {
        VarIndexPair *vid = vector_at(&scope->vars, i);
        StringSlice *name = &vid->var_name;

        if (str.len == name->len && memcmp(str.start, name->start, str.len) == 0) {
            return true;
        }
    }

    return false;
}

const char *symbol_get_map_var(SymbolMap *map, StringSlice str) {
    Scope *scope = map->scope;

    while (scope != nullptr) {
        for (size_t i = 0; i < scope->vars.size; i++) {
            VarIndexPair *vid = vector_at(&scope->vars, i);
            StringSlice *name = &vid->var_name;

            if (str.len == name->len && memcmp(str.start, name->start, str.len) == 0) {
                return ((VarPair *) vector_at(&map->var_map, vid->index))->mangled;
            }
        }
        scope = scope->parent;
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

    return ((ConstPair *) vector_at(&map->const_map, map->const_map.size - 1))->mangled;
}

void symbol_push_scope(SymbolMap *map) {
    Scope *scope = scope_new(map->scope);

    map->scope = scope;
}

void symbol_pop_scope(SymbolMap *map) {
    Scope *scope = map->scope;
    map->scope = scope->parent;

    scope_free(scope);
}

static void next_var_sym(SymbolMap *map, char out[4]) {
    int size = map->var_map.size;

    out[0] = 'w' + (size / (36 * 36));
    out[1] = get_base36_char(size / 36);
    out[2] = get_base36_char(size % 36);
    out[3] = '\0';
}

static void next_const_sym(SymbolMap *map, char out[4]) {
    int size = map->const_map.size;

    out[0] = 'c' + (size / (36 * 36));
    out[1] = get_base36_char(size / 36);
    out[2] = get_base36_char(size % 36);
    out[3] = '\0';
}

static char get_base36_char(unsigned int i) {
    static const char alphanum[] = "0123456789abcdefghijklmnopqrstuvwxyz";
    
    return alphanum[i % 36];
}

static Scope *scope_new(Scope *parent) {
    Scope *scope = xmalloc(sizeof(Scope));
    scope->parent = parent;
    scope->vars = vector_new(sizeof(VarIndexPair));

    return scope;
}

static void scope_free(Scope *scope) {
    vector_free(&scope->vars);
    xfree(scope);
}