# Instructions

## Addressing Modes
Mano's basic computer features `two` addressing modes:
1. Direct address
2. Indirect address

## Instruction Format
Mano's basic computer features `three` instruction formats:
1. Memory Reference
2. Register Reference
3. I/O Instruction

A 16 bit word is divided as follows:
1 bit for I (0 = direct address, 1 = indirect address)
3 bits for opcode
12 bits for address

If the first four bits are 0x7 then the register reference format is used.
If the first four bits are 0xF then the I/O instruction format is used.
Otherwise memory reference format is used.

## Instruction Opcodes

### 1. Memory reference

| Instruction | Direct | Indirect | Description |
| ----------- | -----: | -------: | ----------- |
| `AND`       | `0xxx` | `8xxx` | AND memory word with `AC` |
| `ADD`       | `1xxx` | `9xxx` | Add memory word to `AC` |
| `LDA`       | `2xxx` | `Axxx` | Load memory word into `AC` |
| `STA`       | `3xxx` | `Bxxx` | Store `AC` in memory |
| `BUN`       | `4xxx` | `Cxxx` | Branch unconditionally |
| `BSA`       | `5xxx` | `Dxxx` | Branch and save return address |
| `ISZ`       | `6xxx` | `Exxx` | Increment memory word and skip if zero |

### 2. Register reference

| Instruction | Opcode | Description |
| ----------- | -----: | ----------- |
| `CLA`       | `7800` | Clear `AC` |
| `CLE`       | `7400` | Clear `E` |
| `CMA`       | `7200` | Complement `AC` |
| `CME`       | `7100` | Complement `E` |
| `CIR`       | `7080` | Circulate right `AC` and `E` |
| `CIL`       | `7040` | Circulate left `AC` and `E` |
| `INC`       | `7020` | Increment `AC` |
| `SPA`       | `7010` | Skip next instruction if `AC` is positive |
| `SNA`       | `7008` | Skip next instruction if `AC` is negative |
| `SZA`       | `7004` | Skip next instruction if `AC` is zero |
| `SZE`       | `7002` | Skip next instruction if `E` is zero |
| `HLT`       | `7001` | Halt the computer |

### 3. Input-output

| Instruction | Opcode | Description |
| ----------- | -----: | ----------- |
| `INP`       | `F800` | Input character into `AC` |
| `OUT`       | `F400` | Output character from `AC` |
| `SKI`       | `F200` | Skip next instruction if input flag is set |
| `SKO`       | `F100` | Skip next instruction if output flag is set |
| `ION`       | `F080` | Enable interrupts |
| `IOF`       | `F040` | Disable interrupts |