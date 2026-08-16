# Assembly Language

Mano Basic assembly is a simple, line-oriented language. Each statement occupies exactly one line and is parsed left-to-right through three fields:

```
[LABEL,]  INSTRUCTION  [/ comment]
```

| Field | Required | Description |
|-------|----------|-------------|
| Label | No | A symbolic address followed by a comma |
| Instruction | Yes | A mnemonic with an optional operand |
| Comment | No | Begins with `/`, ignored by the assembler |

Whitespace (spaces, tabs) is not significant except that newlines terminate statements.

---

## Instruction Types

### Memory-Reference Instructions (MRI)

Take a symbolic address as their operand. Appending `I` after the address selects **indirect addressing** — the operand address is treated as a pointer.

```asm
ADD MIN      / Direct:   AC <- AC + M[MIN]
STA DIF I    / Indirect: AC -> M[ M[DIF] ]
```

### Non-Memory-Reference Instructions (non-MRI)

Operate solely on registers and take no operand.

```asm
CMA    / Complement AC
INC    / Increment AC
HLT    / Halt
```

### Pseudo-Instructions

Directives to the assembler rather than real machine instructions.

| Mnemonic | Operand | Effect |
|----------|---------|--------|
| `ORG` | Hex number | Set the location counter to this address |
| `END` | — | Mark the end of the program (required) |
| `DEC` | Signed decimal | Store the value as a 16-bit word at the current location |
| `HEX` | Hex number | Store the value as a 16-bit word at the current location |

---

## Labels

Labels create named symbolic addresses that MRI instructions can reference.

**Rules:**
- Must start with a letter (`A`–`Z`, `a`–`z`)
- May be followed by up to 2 letters or digits (max 3 characters total)
- Must be followed by a comma (`,`) on the same line as an instruction

```asm
LOP, CLE       / 3-character label
X,   HEX 000F  / 1-character label
```

---

## Comments

Begin with `/` and extend to the end of the line.

```asm
LDA SUB   / This entire portion is a comment
```

---

## Grammar

Formal grammar in EBNF:

```ebnf
program     = { statement , NEWLINE } , end , [ NEWLINE ] ;
statement   = [ label ] , instruction ;
label       = IDENTIFIER , "," ;
instruction = mri | nmri | psi ;

mri         = MRI_MNEMONIC , IDENTIFIER , [ "I" ] ;
nmri        = NMRI_MNEMONIC ;
psi         = ( "ORG" , HEX_NUMBER )
            | ( "DEC" , DEC_NUMBER )
            | ( "HEX" , HEX_NUMBER ) ;
end         = "END" ;
```

### Terminal Definitions

| Token | Pattern | Description |
|-------|---------|-------------|
| `MRI_MNEMONIC` | `AND\|ADD\|LDA\|STA\|BUN\|BSA\|ISZ` | Memory-reference mnemonics |
| `NMRI_MNEMONIC` | `CLA\|CLE\|CMA\|CME\|CIR\|CIL\|INC\|SPA\|SNA\|SZA\|SZE\|HLT\|INP\|OUT\|SKI\|SKO\|ION\|IOF` | Non-memory-reference mnemonics |
| `IDENTIFIER` | `[A-Za-z][A-Za-z0-9]{0,2}` | Label or symbolic address (max 3 chars) |
| `DEC_NUMBER` | `-?[0-9]+` | Signed decimal integer |
| `HEX_NUMBER` | `[0-9A-Fa-f]+` | Unsigned hexadecimal integer |

---

## Related

- [Instructions.md](Instructions.md) — Full instruction set with opcodes
- [Assembler.md](Assembler.md) — Assembler CLI and usage