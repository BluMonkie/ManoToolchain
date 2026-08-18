#pragma once
#include <stdint.h>
#include <stdbool.h>

#define MANO_MEMORY_SIZE 4096
#define INVALID_ADDRESS 0xFFFF
#define MANO_ADDRESS_MASK 0xFFF 

typedef uint16_t Word;
typedef uint16_t Address;
typedef uint8_t IO;

typedef enum {
    // Memory reference
    INST_AND,
    INST_ADD,
    INST_LDA,
    INST_STA,
    INST_BUN,
    INST_BSA,
    INST_ISZ,

    // Register reference
    INST_CLA,
    INST_CLE,
    INST_CMA,
    INST_CME,
    INST_CIR,
    INST_CIL,
    INST_INC,
    INST_SPA,
    INST_SNA,
    INST_SZA,
    INST_SZE,
    INST_HLT,

    // I/O
    INST_INP,
    INST_OUT,
    INST_SKI,
    INST_SKO,
    INST_ION,
    INST_IOF,

    INST_INVALID
} InstOpcode;

typedef struct {
    InstOpcode opcode;
    Address operand;
    bool is_indirect;
} Instruction;

typedef struct {
    Address PC;

    Word AC;
    
    IO INPR;
    IO OUTR;

    bool E;
} CPU;

typedef struct {
    Word data[MANO_MEMORY_SIZE];
} Memory;

typedef struct {
    CPU cpu;
    Memory mem;
    bool halted;
} System;

static inline Address address_add(Address a, Address b) {
    return (a + b) & MANO_ADDRESS_MASK;
}

static inline Address address_inc(Address a) {
    return address_add(a, 1);
}

static inline Address address_from_word(Word word) {
    return word & MANO_ADDRESS_MASK;
}
