#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "symbol_map.h"
#include "common/vector.h"
#include "defs.h"

static void next_var_sym(SymbolMap *map, char out[4]);
static void next_const_sym(SymbolMap *map, char out[4]);
static char get_base36_char(unsigned int i);

SymbolMap symbol_map_new() {
    return (SymbolMap) {
        .var_map = vector_new(sizeof(VarPair)),
        .const_map = vector_new(sizeof(ConstPair)),
    };
}

void symbol_map_free(SymbolMap *map) {
    vector_free(&map->var_map);
    vector_free(&map->const_map);
}

bool symbol_define_var(SymbolMap *map, StringSlice str) {
    if (symbol_exists(map, str)) return false;

    VarPair pair = {
        .var_name = str
    };

    next_var_sym(map, pair.mangled);
    vector_push(&map->var_map, &pair);

    return true;
}

bool symbol_exists(SymbolMap *map, StringSlice str) {
    return symbol_get_map_var(map, str) != NULL;
}

const char *symbol_get_map_var(SymbolMap *map, StringSlice str) {
    for (size_t i = 0; i < map->var_map.size; i++) {
        VarPair *var = vector_at(&map->var_map, i);

        if (str.len == var->var_name.len && memcmp(str.start, var->var_name.start, str.len) == 0) {
            return var->mangled;
        }
    }

    return NULL;
}

const char *symbol_get_map_const(SymbolMap *map, int num) {
    for (size_t i = 0; i < map->const_map.size; i++) {
        ConstPair *pair = vector_at(&map->const_map, i);

        if (pair->value == num) {
            return pair->mangled;
        }
    }

    ConstPair pair = {
        .value = num
    };

    next_const_sym(map, pair.mangled);
    vector_push(&map->const_map, &pair);

    return ((ConstPair *) vector_at(&map->const_map, map->const_map.size - 1))->mangled;
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