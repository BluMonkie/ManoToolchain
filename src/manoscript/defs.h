#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

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

typedef enum {
    INST_MEM_AND,
    INST_MEM_ADD,
    INST_MEM_LDA,
    INST_MEM_STA,
    INST_MEM_BUN,
    INST_MEM_BSA,
    INST_MEM_ISZ,

    INST_REG_CLA,
    INST_REG_CLE,
    INST_REG_CMA,
    INST_REG_CME,
    INST_REG_CIR,
    INST_REG_CIL,
    INST_REG_INC,
    INST_REG_SPA,
    INST_REG_SNA,
    INST_REG_SZA,
    INST_REG_SZE,
    INST_REG_HLT,

    INST_IO_INP,
    INST_IO_OUT,
    INST_IO_SKI,
    INST_IO_SKO,
    INST_IO_ION,
    INST_IO_IOF,

    INST_PSI_ORG,
    INST_PSI_DEC,
    INST_PSI_HEX,
    INST_PSI_END,
} ManoInstType;

typedef struct {
    const char *label;
    ManoInstType type;
    bool indirect;
    const char *argument;
} ManoInst;