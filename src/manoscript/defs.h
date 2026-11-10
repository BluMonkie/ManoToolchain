#pragma once

#include <stddef.h>
#include <stdint.h>

typedef struct {
    const char *start;
    size_t len;
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
    TOKEN_LEFT_PAREN, TOKEN_RIGHT_PAREN,

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

static inline const char *token_type_name(TokenType type) {
    static const char *names[] = {
        [TOKEN_PLUS]          = "+",
        [TOKEN_MINUS]         = "-",
        [TOKEN_STAR]          = "*",
        [TOKEN_PERCENT]       = "%",
        [TOKEN_SLASH]         = "/",
        [TOKEN_EQUAL]         = "=",
        [TOKEN_EQUAL_EQUAL]   = "==",

        [TOKEN_AT]            = "@",

        [TOKEN_IDENTIFIER]    = "identifier",
        [TOKEN_NUMBER]        = "number",

        [TOKEN_VAR]           = "var",

        [TOKEN_COLON]         = ":",
        [TOKEN_SEMICOLON]     = ";",
        [TOKEN_LEFT_PAREN]    = "(",
        [TOKEN_RIGHT_PAREN]   = ")",

        [TOKEN_EOF]           = "EOF",
    };

    if ((size_t)type >= sizeof(names) / sizeof(names[0]))
        return "unknown";

    return names[type] ? names[type] : "unknown";
}