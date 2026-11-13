#pragma once

typedef enum {
    INT_CMP_LS,
    INT_CMP_GR,
    INT_CMP_LS_EQ,
    INT_CMP_GR_EQ,
    INT_CMP_EQ,
    INT_CMP_NEQ,
    INT_COUNT,
} Intrinsic;

typedef struct {
    bool enabled;
    Intrinsic intr;
    const char *label;
} IntrinsicEntry;