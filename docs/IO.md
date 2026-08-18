# I/O System

The Mano Basic Computer performs I/O through two dedicated 8-bit registers (`INPR` and `OUTR`) and two 1-bit synchronisation flags (`FGI` and `FGO`). Two modes of operation are supported: **programmed I/O**, where the CPU polls the flags in a busy-wait loop, and **interrupt-driven I/O**, where ready device flags can generate an interrupt request when interrupts are enabled.

---

## Registers

| Symbol | Bits | Name | Description |
|--------|-----:|------|-------------|
| `INPR` | 8 | Input Register | Holds the last character received from the input device |
| `OUTR` | 8 | Output Register | Holds the character to be sent to the output device |

`INPR` and `OUTR` are **not** connected to the common bus and cannot be addressed directly by memory-reference instructions. They are accessed exclusively through the `INP` and `OUT` I/O instructions.

---

## Flags

Two 1-bit flags synchronise the CPU with the peripherals:

| Flag | Name | Set by | Cleared by | Meaning |
|------|------|--------|------------|---------|
| `FGI` | Input Flag | Input device | `INP` instruction | A new character is ready in `INPR` |
| `FGO` | Output Flag | Output device | `OUT` instruction | The output device is ready for a new character |

The CPU tests these flags with `SKI` (skip if `FGI = 1`) and `SKO` (skip if `FGO = 1`).

## Initial State

On reset:

| Flag | Initial value |
|------|--------------:|
| `FGI` | `0` |
| `FGO` | `1` |
| `IEN` | `0` |

The input device initially has no character available, while the output device is initially ready to accept a character.

---

## I/O Instructions

I/O instructions are encoded with the top nibble `0xF`. Their lower 12 bits select the operation.

| Mnemonic | Opcode | Operation | Side Effects |
|----------|:------:|-----------|--------------|
| `INP` | `F800` | `AC[7:0] ← INPR` | Clears `FGI` (`FGI ← 0`) |
| `OUT` | `F400` | `OUTR ← AC[7:0]` | Clears `FGO` (`FGO ← 0`) |
| `SKI` | `F200` | Skip next instruction if `FGI = 1` | — |
| `SKO` | `F100` | Skip next instruction if `FGO = 1` | — |
| `ION` | `F080` | `IEN ← 1` (enable interrupts) | — |
| `IOF` | `F040` | `IEN ← 0` (disable interrupts) | — |

> **Note:** `INP` copies only the low-order 8 bits of `INPR` into `AC`; the upper 8 bits of `AC` are left unchanged. Clear `AC` first with `CLA` if a clean 8-bit value is required.

---

## Input

The input device operates **asynchronously**: it sets `FGI` and writes a character into `INPR` independently of the CPU. Programs should check `FGI` before executing `INP` to ensure a newly received character is available — either by polling in a loop (programmed I/O) or by entering an ISR triggered by the flag (interrupt-driven I/O). Executing `INP` when `FGI = 0` will simply read whatever stale value is currently in `INPR`.

### Programmed-I/O Input Loop

```manoasm
CIF, SKI        / Skip next if FGI = 1 (input ready)
     BUN CIF    / Not ready; loop back and poll again
     INP        / AC[7:0] <- INPR;  FGI <- 0
```

The character is now in the lower byte of `AC`.

### Behaviour of `INP`

```
INP:  AC[7:0] ← INPR
      FGI     ← 0
```

Clearing `FGI` signals to the input device that the CPU has consumed the character, allowing it to accept and signal the next one.

---

## Output

The output device also operates **asynchronously**: it reads `OUTR` and renders the character independently of the CPU. Programs should check `FGO` before executing `OUT` to avoid overwriting output data that has not yet been consumed by the device — again either by polling or via an interrupt.

> **Note:** A character is **not** automatically echoed to the terminal when received. To display input, explicitly execute an `OUT` instruction after `INP`.

### Programmed-I/O Output Loop

```manoasm
COF, SKO        / Skip next if FGO = 1 (output device ready)
     BUN COF    / Not ready; loop back and poll again
     OUT        / OUTR <- AC[7:0];  FGO <- 0
```

### Behaviour of `OUT`

```
OUT:  OUTR ← AC[7:0]
      FGO  ← 0
```

Clearing `FGO` signals to the output device that new data is in `OUTR`. The device reads it and sets `FGO` again when it is ready for the next character.

---

## Examples

### Read one character and echo it

```manoasm
     ORG 0
     / --- Input loop ---
CIF, SKI        / Wait for input flag
     BUN CIF
     INP         / AC <- char from INPR; FGI <- 0
     / --- Output loop ---
COF, SKO        / Wait for output device ready
     BUN COF
     OUT         / OUTR <- AC[7:0]; FGO <- 0
     STA CHR     / Save character to memory
     HLT
CHR, DEC 0      / Storage for the character
     END
```

### Read and echo characters into a buffer

```manoasm
     ORG 10
LOP, SKI         / Wait for input
     BUN LOP
     CLA
     INP          / AC <- next character; FGI <- 0

     / Echo the character
ECH, SKO
     BUN ECH
     OUT

     / Store at address held in IDX, then advance the pointer
     STA IDX I
     ISZ IDX
     BUN LOP      / Loop until IDX (16-bit word) wraps from 0xFFFF to 0x0000

     HLT

IDX, HEX 050     / Write pointer (points to start of BUF)

     ORG 50
BUF, DEC 0       / Buffer begins here
     END
```

> This program runs until the 16-bit word stored in `IDX` wraps from `0xFFFF` to `0x0000`, at which point `ISZ` skips `BUN LOP` and execution falls through to `HLT`. During indirect addressing, only the low-order 12 bits of `IDX` are used as the effective address, so the buffer address itself wraps every `0x1000` increments — long before the `ISZ` skip condition is reached.

---

## Interrupt-Driven I/O

As an alternative to polling, the Mano computer supports a single interrupt channel controlled by the **Interrupt Enable** flag `IEN`.

### Enabling Interrupts

```manoasm
ION     / IEN <- 1   (interrupts enabled)
IOF     / IEN <- 0   (interrupts disabled)
```

### Interrupt Trigger Condition

An interrupt request is pending if **both** of the following are true:

1. `IEN = 1`
2. At least one device flag is set (`FGI = 1` or `FGO = 1`)

When an interrupt request is pending, the control unit performs the interrupt cycle before beginning the next instruction.

### Interrupt Sequence (hardware)

| Step | Action |
|------|--------|
| 1 | Save return address: `M[0] ← PC` |
| 2 | Jump to ISR: `PC ← 1` |
| 3 | Disable further interrupts: `IEN ← 0` |

The Interrupt Service Routine (ISR) begins at **memory address 1**. It is responsible for:

- Saving and restoring `AC` to and from a dedicated memory location.
- Determining which device triggered the interrupt (`SKI`/`SKO`).
- Servicing the device (`INP` or `OUT`).
- Re-enabling interrupts (`ION`) before returning.
- Returning via an indirect branch through the saved return address at location `0`.

### Minimal ISR Skeleton

```manoasm
     ORG 0
RET, DEC 0       / Return address saved here by hardware

     ORG 1        / ISR entry point
     STA SAC      / Save AC to memory
     SKI          / Was it an input interrupt?
     BUN OTT      / No — check output
     / --- Handle input ---
     INP
     STA CBF      / Store received character
     BUN DON
OTT, SKO          / Was it an output interrupt?
     BUN DON      / No device ready (spurious); just return
     / --- Handle output ---
     LDA ONX      / Load next character to send
     OUT
DON, LDA SAC     / Restore AC
     ION          / Re-enable interrupts
     BUN RET I    / Return (indirect branch through address in location 0)

SAC, DEC 0       / Saved accumulator
CBF, DEC 0       / Last received character
ONX, DEC 0       / Next character to output
     END
```

> **Note:** This is a conceptual skeleton, not a production-ready driver. In particular, the output handler executes whenever `FGO = 1`, which may happen immediately after `ION` if the output device is already ready. If `ONX` is uninitialised (or zero), this will output a NUL character. A practical driver would maintain software state indicating whether output is pending. Note also that if both `FGI` and `FGO` are set when the ISR runs, only the input device is serviced; the remaining `FGO = 1` flag will trigger another interrupt request after `ION` re-enables interrupts. Finally, the Mano Basic Computer provides only a single global interrupt enable flag (`IEN`), so `IOF` disables interrupts for both input and output rather than selectively disabling output interrupts.

---

## Related

- [Registers.md](Registers.md) — Full register and flag descriptions
- [Instructions.md](Instructions.md) — I/O instruction opcodes and semantics
- [Architecture.md](Architecture.md) — Fetch–decode–execute cycle and interrupt system overview