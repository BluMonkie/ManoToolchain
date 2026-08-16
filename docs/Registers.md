# Registers

The Mano Basic Computer has 9 registers grouped by their role.

---

## Register Summary

| Symbol | Bits | Name | Role |
|--------|-----:|------|------|
| `AC` | 16 | Accumulator | Primary arithmetic/logic register; operand for most instructions |
| `E` | 1 | Extended Accumulator | Carry/overflow bit; rotated with `AC` by `CIR`/`CIL` |
| `DR` | 16 | Data Register | Temporarily holds a word read from memory |
| `AR` | 12 | Address Register | Supplies the address to the memory unit |
| `IR` | 16 | Instruction Register | Holds the currently executing instruction word |
| `PC` | 12 | Program Counter | Address of the next instruction to fetch |
| `TR` | 16 | Temporary Register | Scratch register used during interrupt handling |
| `INPR` | 8 | Input Register | Holds the last character received from input |
| `OUTR` | 8 | Output Register | Holds the character to be sent to output |

---

## I/O Flags

Two 1-bit flags synchronise the CPU with I/O devices:

| Flag | Name | Set when |
|------|------|----------|
| `FGI` | Input Flag | Input device has a new character ready in `INPR` |
| `FGO` | Output Flag | Output device is ready to accept a new character from `OUTR` |

The `SKI` and `SKO` instructions test these flags. The `ION`/`IOF` instructions control the interrupt enable flag (`IEN`).

---

## Register Transfer Notation

Register transfer language (RTL) is used throughout the documentation to describe what each instruction does.

| Symbol | Meaning |
|--------|---------|
| `R` | Register name (e.g. `AC`, `DR`) |
| `R[i]` | Bit `i` of register `R` |
| `R[i:j]` | Bits `i` down to `j` of register `R` |
| `M[X]` | Contents of memory at address `X` |
| `R ← expr` | Transfer (assign) `expr` into register `R` |
| `~R` | Bitwise complement of `R` |
| `∧` | Bitwise AND |
| `;` | Sequential micro-operations (executed left to right) |

**Example:**

```
BSA X:  M[X] ← PC,  PC ← X + 1
```

Saves the return address in `M[X]`, then jumps to the instruction after `X`.

---

## Related

- [Instructions.md](Instructions.md) — Full instruction set with opcodes
