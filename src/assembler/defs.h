#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define INVALID_ADDRESS UINT16_MAX

typedef uint16_t Address;

typedef enum {
    // Memory-reference instructions
    TOKEN_AND, TOKEN_ADD, TOKEN_LDA, 
    TOKEN_STA, TOKEN_BUN, TOKEN_BSA, TOKEN_ISZ,

    // Register-reference instructions
    TOKEN_CLA, TOKEN_CLE, TOKEN_CMA, TOKEN_CME,
    TOKEN_CIR, TOKEN_CIL, TOKEN_INC, TOKEN_SPA,
    TOKEN_SNA, TOKEN_SZA, TOKEN_SZE, TOKEN_HLT,

    // I/O instructions
    TOKEN_INP, TOKEN_OUT, TOKEN_SKI,
    TOKEN_SKO, TOKEN_ION, TOKEN_IOF,

    // Assembly directives
    TOKEN_ORG, TOKEN_END, TOKEN_DEC, TOKEN_HEX, TOKEN_I,

    // Literals and identifiers
    TOKEN_IDENTIFIER, TOKEN_DEC_NUMBER, TOKEN_HEX_NUMBER,

    // Punctuation
    TOKEN_COMMA, TOKEN_NEWLINE,

    // Special
    TOKEN_EOF
} TokenType;

typedef struct {
    TokenType type;
    size_t lexeme_start;
    size_t lexeme_end;
    int16_t number;
} Token;

typedef enum {
    INST_MEM_AND = 0, 
    INST_MEM_ADD = 1,
    INST_MEM_LDA = 2, 
    INST_MEM_STA = 3,
    INST_MEM_BUN = 4, 
    INST_MEM_BSA = 5,
    INST_MEM_ISZ = 6,

    INST_REG_CLA = 0x7800, 
    INST_REG_CLE = 0x7400,
    INST_REG_CMA = 0x7200, 
    INST_REG_CME = 0x7100,
    INST_REG_CIR = 0x7080, 
    INST_REG_CIL = 0x7040,
    INST_REG_INC = 0x7020, 
    INST_REG_SPA = 0x7010,
    INST_REG_SNA = 0x7008,
    INST_REG_SZA = 0x7004,
    INST_REG_SZE = 0x7002,
    INST_REG_HLT = 0x7001,

    INST_IO_INP = 0xF800,
    INST_IO_OUT = 0xF400,
    INST_IO_SKI = 0xF200,
    INST_IO_SKO = 0xF100,
    INST_IO_ION = 0xF080,
    INST_IO_IOF = 0xF040,

    INST_PSI_ORG,
    INST_PSI_DEC,
    INST_PSI_HEX,
    INST_PSI_END,

    INST_INVALID
} Instruction;

typedef struct {
    Token label;
    Instruction instruction;
    bool indirect;

    Address location;

    int16_t operand;
    bool operand_pending;
    Token *symbolic_address;
} Statement;