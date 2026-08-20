# Assembler (`manoasm`)

`manoasm` translates Mano Basic assembly source files (`.masm`) into a raw binary memory image (`.mano`) that can be loaded and executed by the emulator.

---

## Usage

```sh
manoasm [options] <input_file.masm>
```

### Options

| Option | Description |
|--------|-------------|
| `--help` | Print usage information |
| `--version` | Print version information |
| `--output <file>` | Output file path (default: `<input>.mano`) |
| `--dump-tokens` | Write token stream to `<input>_tokens.txt` |
| `--dump-ast` | Write parsed statement list to `<input>_ast.txt` |
| `--dump-symbols` | Write symbol table to `<input>_symbols.txt` |

### Quick Start

```sh
# Assemble a file (outputs subtract.mano alongside the source)
manoasm tests/assembler/subtract.masm

# Specify a custom output path
manoasm --output build/out.mano tests/assembler/subtract.masm

# Dump all intermediate representations for debugging
manoasm --dump-tokens --dump-symbols --dump-ast tests/assembler/multiply.masm
```

---

## Pipeline

The assembler processes source in three sequential stages:

```
Source (.masm)
      │
      ▼
  ┌────────┐
  │ Lexer  │  Scans characters and produces a flat list of tokens
  └────────┘
      │
      ▼
  ┌────────┐
  │ Parser │  Two-pass parse — first pass assigns addresses to labels,
  │        │  second pass resolves all symbolic references
  └────────┘
      │
      ▼
  ┌─────────┐
  │ Encoder │  Emits 16-bit machine words into a 4096-word memory image
  └─────────┘
      │
      ▼
Binary (.mano)
```

The two-pass approach means forward references (using a label before it is defined) are fully supported.

---

## Output Format

The output is a flat **big-endian binary** file representing the entire 4096-word memory space:

- **Size:** always 8192 bytes (4096 words × 2 bytes each)
- **Byte order:** big-endian — most significant byte first
- **Unused words:** filled with `0x0000`

Words are written at the addresses set by `ORG` directives; all other locations default to zero.

---

## Sample Programs

### Subtraction

Computes `83 − (−23) = 106` using two's complement negation.

```asm
ORG 100      / Program starts at address 0x100

LDA SUB      / AC <- M[SUB]  (load subtrahend)
CMA          / AC <- ~AC     (bitwise complement)
INC          / AC <- AC + 1  (complete two's complement)
ADD MIN      / AC <- AC + M[MIN]
STA DIF      / M[DIF] <- AC  (store result)
HLT

MIN, DEC 83  / Minuend
SUB, DEC -23 / Subtrahend
DIF, HEX 0   / Result stored here

END
```

### Multiplication

Multiplies `X` (= 15) by `Y` (= 11) using 8 iterations of shift-and-add.

The algorithm shifts the multiplier right one bit per iteration. When the shifted-out bit is 1, the multiplicand (left-shifted by the same amount) is added to the partial product.

```asm
ORG 100

LOP, CLE          / E <- 0
     LDA Y        / AC <- Y (multiplier)
     CIR          / Shift Y right; LSB goes into E
     STA Y        / Store shifted multiplier
     SZE          / Skip next if E = 0
     BUN ONE      / E = 1: add multiplicand
     BUN ZRO      / E = 0: skip addition

ONE, LDA X        / AC <- X (multiplicand)
     ADD P        / AC <- AC + P (partial product)
     STA P        / P <- AC

ZRO, CLE          / E <- 0
     LDA X        / AC <- X
     CIL          / Shift X left (scale multiplicand)
     STA X        / Store shifted multiplicand
     ISZ CTR      / CTR++; skip next if CTR = 0
     BUN LOP      / Repeat loop
     HLT          / Done

CTR, DEC -8       / Loop counter (counts up to 0 over 8 iterations)
X,   HEX 000F     / Multiplicand = 15
Y,   HEX 000B     / Multiplier   = 11
P,   HEX 0        / Accumulated product

END
```

---

## Related

- [Assembly.md](Assembly.md) — Syntax, grammar, labels, pseudo-instructions
- [Instructions.md](Instructions.md) — Full instruction set with opcodes
- [Registers.md](Registers.md) — CPU register descriptions
- [Emulator.md](Emulator.md) — Running the assembled binary
