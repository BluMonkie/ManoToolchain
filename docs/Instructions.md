# Instructions

---

## Instruction Format

Each instruction is encoded as a **16-bit word** divided into three fields:

```
 15  14  13  12  11                          0
┌───┬───────────┬──────────────────────────────┐
│ I │  Opcode   │           Address            │
│(1)│  (3 bits) │          (12 bits)           │
└───┴───────────┴──────────────────────────────┘
```

The top 4 bits (`I` + opcode) determine the instruction format:

| Bits [15:12] | Format | Notes |
|:------------:|--------|-------|
| `0x0`–`0x6` | Memory Reference | I=0 (direct) |
| `0x8`–`0xE` | Memory Reference | I=1 (indirect) |
| `0x7` | Register Reference | Bits [11:0] encode the operation |
| `0xF` | I/O | Bits [11:0] encode the operation |

---

## Addressing Modes

Memory-reference instructions support two addressing modes, selected by the **I** bit:

| Mode | I bit | Effective Address |
|------|:-----:|-------------------|
| Direct | `0` | The 12-bit address field |
| Indirect | `1` | The word stored at the 12-bit address field |

Indirect mode is specified in assembly by appending `I` to the instruction:

```asm
LDA X    / Direct:   AC <- M[X]
LDA X I  / Indirect: AC <- M[ M[X] ]
```

---

## Instruction Set

### Memory Reference

The 12-bit address field (`xxx`) holds the operand's memory address.

| Mnemonic | Direct | Indirect | Operation |
|----------|:------:|:--------:|-----------|
| `AND` | `0xxx` | `8xxx` | `AC ← AC ∧ M[X]` |
| `ADD` | `1xxx` | `9xxx` | `AC ← AC + M[X]` (carry → E) |
| `LDA` | `2xxx` | `Axxx` | `AC ← M[X]` |
| `STA` | `3xxx` | `Bxxx` | `M[X] ← AC` |
| `BUN` | `4xxx` | `Cxxx` | `PC ← X` (unconditional branch) |
| `BSA` | `5xxx` | `Dxxx` | `M[X] ← PC, PC ← X+1` (call subroutine) |
| `ISZ` | `6xxx` | `Exxx` | `M[X] ← M[X]+1; if M[X]=0: skip` |

### Register Reference

Bit pattern `0111` (`0x7`) in the top 4 bits selects register-reference format. The remaining 12 bits each correspond to one micro-operation, allowing multiple operations to be combined in a single instruction word.

| Mnemonic | Opcode | Operation |
|----------|:------:|-----------|
| `CLA` | `7800` | `AC ← 0` |
| `CLE` | `7400` | `E ← 0` |
| `CMA` | `7200` | `AC ← ~AC` |
| `CME` | `7100` | `E ← ~E` |
| `CIR` | `7080` | Rotate `AC`–`E` right by 1 |
| `CIL` | `7040` | Rotate `AC`–`E` left by 1 |
| `INC` | `7020` | `AC ← AC + 1` |
| `SPA` | `7010` | Skip next if `AC > 0` |
| `SNA` | `7008` | Skip next if `AC < 0` |
| `SZA` | `7004` | Skip next if `AC = 0` |
| `SZE` | `7002` | Skip next if `E = 0` |
| `HLT` | `7001` | Halt |

### I/O

Bit pattern `1111` (`0xF`) in the top 4 bits selects I/O format.

| Mnemonic | Opcode | Operation |
|----------|:------:|-----------|
| `INP` | `F800` | `AC[7:0] ← INPR` (read input character) |
| `OUT` | `F400` | `OUTR ← AC[7:0]` (write output character) |
| `SKI` | `F200` | Skip next if input flag (`FGI`) is set |
| `SKO` | `F100` | Skip next if output flag (`FGO`) is set |
| `ION` | `F080` | Enable interrupt system (`IEN ← 1`) |
| `IOF` | `F040` | Disable interrupt system (`IEN ← 0`) |

---

## Related

- [Registers.md](Registers.md) — CPU register descriptions
- [Assembly.md](Assembly.md) — Assembly syntax and grammar