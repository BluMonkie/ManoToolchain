#include <string.h>
#include <stdio.h>

#include "symbols.h"
#include "common/vector.h"

SymbolTable symbol_table_new(char *src) {
    Vector vec = vector_new(sizeof(SymbolLocation));
    
    return (SymbolTable) {
        .src = src,
        .symbols = vec,
        .had_error = false,
    };
}

bool is_symbol_defined(SymbolTable *table, Token identifier) {
    return symbol_find(table, identifier) != NULL;
}

void symbol_define(SymbolTable* table, Token identifier, Address location) {
    if (is_symbol_defined(table, identifier)) {
        table->had_error = true;
        fprintf(stderr, "ERROR: Label %.*s is already defined.\n", (int)(identifier.lexeme_end - identifier.lexeme_start), table->src + identifier.lexeme_start);
        return;
    }

    vector_push(&table->symbols, &((SymbolLocation) {.identifier = identifier, .location = location}));
}

const SymbolLocation *symbol_find(SymbolTable* table, Token identifier) {
    size_t identifier_len = identifier.lexeme_end - identifier.lexeme_start;

    for (size_t i = 0; i < table->symbols.size; i++) {
        const SymbolLocation *sym = vector_at_const(&table->symbols, i);

        size_t symbol_len = sym->identifier.lexeme_end - sym->identifier.lexeme_start;

        if (symbol_len != identifier_len) {
            continue;
        }

        if (memcmp(table->src + sym->identifier.lexeme_start, table->src + identifier.lexeme_start, identifier_len) == 0) {
            return sym;
        }
    }

    return NULL;
}

Address symbol_locate(SymbolTable* table, Token identifier) {
    const SymbolLocation *sym = symbol_find(table, identifier);

    if (sym != NULL) {
        return sym->location;
    }

    table->had_error = true;
    fprintf(stderr, "ERROR: Label %.*s is not defined.\n", (int)(identifier.lexeme_end - identifier.lexeme_start), table->src + identifier.lexeme_start);
    return INVALID_ADDRESS;
}