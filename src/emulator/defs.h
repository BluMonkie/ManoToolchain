#pragma once
#include <stdint.h>
#include <stdbool.h>

#define MEMORY_WORDS 4096

typedef uint16_t Word;
typedef uint16_t Address;
typedef uint8_t IO;

typedef struct {
    Address PC;
    Address AR;

    Word DR;
    Word IR;
    Word AC;
    Word TR;
    
    IO INPR;
    IO OUTR;

    bool E;
} CPU;

typedef struct {
    Word data[MEMORY_WORDS];
} Memory;

typedef struct {
    CPU cpu;
    Memory mem;
} System;