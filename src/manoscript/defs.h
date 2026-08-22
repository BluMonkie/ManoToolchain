#pragma once

#include <stddef.h>
#include <stdint.h>

typedef struct {
    const char *start;
    const size_t len;
} StringSlice;

typedef enum {
    // Operators
    // Single character
    TOKEN_PLUS, TOKEN_MINUS, TOKEN_STAR, TOKEN_PERCENT,
    // One or two characters
    TOKEN_SLASH,
    TOKEN_EQUAL, TOKEN_EQUAL_EQUAL,
    
    // Special operators
    TOKEN_AT,

    // Literals and identifiers
    TOKEN_IDENTIFIER, TOKEN_NUMBER,

    // Keywords
    TOKEN_VAR,

    // Punctuation
    TOKEN_COLON, TOKEN_SEMICOLON,

    // Special tokens
    TOKEN_EOF,
} TokenType;

typedef struct {
    TokenType type;
    StringSlice lexeme;
    union {
        int64_t number;
    } data;
} Token;

