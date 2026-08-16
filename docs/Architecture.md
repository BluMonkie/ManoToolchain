# Architecture

The Mano Basic Computer is a 16-bit, accumulator-based Von Neumann machine designed as a teaching tool for computer organisation. It is described in *Computer System Architecture* by M. Morris Mano.

---

## Memory

- **Size:** 4096 words
- **Word width:** 16 bits
- **Addressing:** 12-bit address bus (hence 2¹² = 4096 addressable locations)
- **Organisation:** a flat, word-addressed space shared by instructions and data

Memory is accessed through the **Address Register (AR)** and **Data Register (DR)**: the CPU loads AR with the target address, then the memory unit places the word into DR (read) or writes DR into memory (write).

---

## Common Bus

All data transfers between registers are routed through a single **16-bit common bus**. Only one source may drive the bus at a time. The bus connects:

```
 ┌────┐  ┌────┐  ┌────┐  ┌────┐  ┌────┐  ┌────┐  ┌────────┐
 │ AR │  │ PC │  │ DR │  │ AC │  │ IR │  │ TR │  │ Memory │
 └──┬─┘  └──┬─┘  └──┬─┘  └──┬─┘  └──┬─┘  └──┬─┘  └───┬────┘
    └────────┴────────┴───────┴───────┴───────┴─────────┘
                         Common Bus (16 bits)
```

The bus width is 16 bits, but AR and PC are only 12 bits wide — they take the low-order 12 bits when loaded from the bus.

---

## Registers

| Symbol | Bits | Name | Role |
|--------|-----:|------|------|
| `AC` | 16 | Accumulator | Primary arithmetic/logic register |
| `E` | 1 | Extended Accumulator | Carry/overflow bit; rotates with `AC` |
| `DR` | 16 | Data Register | Holds operand words fetched from memory |
| `AR` | 12 | Address Register | Supplies addresses to the memory unit |
| `IR` | 16 | Instruction Register | Holds the currently executing instruction |
| `PC` | 12 | Program Counter | Address of the next instruction |
| `TR` | 16 | Temporary Register | Scratch register used during interrupts |
| `INPR` | 8 | Input Register | Character received from the input device |
| `OUTR` | 8 | Output Register | Character to be sent to the output device |

See [Registers.md](Registers.md) for I/O flags and register-transfer notation.

---

## Instruction Formats

A 16-bit instruction word uses one of three formats, selected by the top 4 bits:

```
 15  14  13  12  11                          0
┌───┬───────────┬──────────────────────────────┐
│ I │  Opcode   │           Address            │
│(1)│  (3 bits) │          (12 bits)           │
└───┴───────────┴──────────────────────────────┘
  Memory Reference (top nibble 0x0–0x6, 0x8–0xE)

┌───────────────┬──────────────────────────────┐
│  0111 (0x7)   │      Operation bits          │
│               │         (12 bits)            │
└───────────────┴──────────────────────────────┘
  Register Reference

┌───────────────┬──────────────────────────────┐
│  1111 (0xF)   │      Operation bits          │
│               │         (12 bits)            │
└───────────────┴──────────────────────────────┘
  I/O Instruction
```

| Format | Count | Top nibble | Operand |
|--------|:-----:|:----------:|---------|
| Memory Reference | 7 | `0x0`–`0x6`, `0x8`–`0xE` | 12-bit memory address |
| Register Reference | 12 | `0x7` | Operation bits (combinable) |
| I/O | 6 | `0xF` | Operation bits |

The **I** bit (bit 15) in memory-reference instructions selects the addressing mode.

See [Instructions.md](Instructions.md) for the full opcode table with operation semantics.

---

## Addressing Modes

Memory-reference instructions support two modes:

| Mode | I bit | Effective Address |
|------|:-----:|-------------------|
| Direct | `0` | The 12-bit address field |
| Indirect | `1` | The word stored at the 12-bit address field |

Indirect mode enables pointer-based access. In assembly, it is written by appending `I`:

```asm
LDA X    / AC <- M[X]       (direct)
LDA X I  / AC <- M[ M[X] ]  (indirect)
```

---

## Fetch–Decode–Execute Cycle

Each instruction goes through three phases:

### 1. Fetch
```
AR <- PC
IR <- M[AR],  PC <- PC + 1
```
The instruction at the address in `PC` is loaded into `IR`, then `PC` is incremented.

### 2. Decode
The processor examines bits `[14:12]` of `IR` (the opcode) and bit `[15]` (I):
- If opcode = `111` → register-reference instruction
- If opcode = `111` and I = `1` → I/O instruction
- Otherwise → memory-reference instruction

For memory-reference instructions, the address is also resolved:
```
AR <- IR[11:0]       / load address field
if I = 1: AR <- M[AR]  / indirect: follow the pointer
```

### 3. Execute
The operation encoded by the instruction is carried out (ALU operation, memory read/write, branch, etc.).

---

## I/O System

The Mano computer uses **programmed I/O** — the CPU explicitly polls device flags rather than receiving interrupts for every character.

Two 1-bit flags synchronise the CPU with peripherals:

| Flag | Name | Meaning |
|------|------|---------|
| `FGI` | Input Flag | Set by the input device when `INPR` holds a new character |
| `FGO` | Output Flag | Set by the output device when it is ready for a new character |

**Typical input loop:**
```asm
SKI          / Skip next if FGI = 1 (input ready)
BUN SKI_LOC  / Not ready; keep polling
INP          / AC[7:0] <- INPR; FGI <- 0
```

**Typical output loop:**
```asm
SKO          / Skip next if FGO = 1 (output ready)
BUN SKO_LOC  / Not ready; keep polling
OUT          / OUTR <- AC[7:0]; FGO <- 0
```

---

## Interrupt System

The computer supports a single **interrupt enable** flag (`IEN`), controlled by `ION` and `IOF`.

When `IEN = 1` and a device flag (`FGI` or `FGO`) is set at the end of an execute cycle, an interrupt is triggered:

1. The current `PC` is saved in memory location `0`.
2. `PC` is set to `1` (the start of the interrupt service routine).
3. Interrupts are disabled (`IEN <- 0`).

The interrupt service routine is responsible for re-enabling interrupts (`ION`) before returning. The `TR` register is typically used to save and restore `AC` during the ISR.

---

## Related

- [Registers.md](Registers.md) — Detailed register and flag descriptions
- [Instructions.md](Instructions.md) — Full instruction set with opcodes and semantics
- [Assembly.md](Assembly.md) — Assembly language syntax and grammar
