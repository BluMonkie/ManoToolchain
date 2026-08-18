# Mano Basic Computer Toolchain

A C implementation of a toolchain for the **Mano Basic Computer** — a classic educational computer architecture described in *Computer System Architecture* by M. Morris Mano. The goal is to build a complete environment for writing, assembling, and running Mano Basic programs.

---

## The Mano Basic Computer

The Mano Basic Computer is a 16-bit, accumulator-based architecture designed as a teaching tool. It has a 12-bit address space giving it **4096 words** of memory, a small set of registers, and 25 instructions across three formats:

| Format | Instruction Count | Selected by |
|--------|:-----------------:|-------------|
| Memory Reference | 7 | Top 4 bits = `0x0`–`0x6` / `0x8`–`0xE` |
| Register Reference | 12 | Top 4 bits = `0x7` |
| I/O | 6 | Top 4 bits = `0xF` |

Its simplicity makes it an ideal target for understanding the fundamentals of computer organisation: fetch-decode-execute cycles, two-pass assembly, and register-transfer-level semantics.

See [docs/Architecture.md](docs/Architecture.md) for the full architecture reference.

---

## Sub-projects

| Sub-project | Binary | Status | Description |
|-------------|--------|:------:|-------------|
| **Assembler** | `bin/manoasm` | ✅ Complete | Translates `.masm` source into a binary memory image (`.mano`) |
| **Emulator** | `bin/manoemu` | 🚧 In progress | Loads and executes `.mano` binaries |

---

## Project Structure

```
mano/
├── Makefile
├── docs/                   # Documentation
│   ├── Architecture.md
│   ├── Assembler.md
│   ├── Assembly.md
│   ├── Instructions.md
│   └── Registers.md
├── src/
│   ├── common/             # Shared utilities (file I/O, dynamic array)
│   ├── assembler/          # Assembler source
│   └── emulator/           # Emulator source (WIP)
├── tests/
│   └── assembler/          # Sample .masm programs
└── bin/                    # Compiled binaries (generated)
```

---

## Building

**Requirements:** GCC with C23 support (`-std=c23`), GNU Make.

```sh
# Build everything
make

# Build a specific sub-project
make assembler
make emulator

# Clean build artifacts
make clean
```

Binaries are placed in `bin/` after a successful build.

---

## Quick Start

Write a source file:

```asm
/ subtract.masm — computes 83 - (-23) = 106

ORG 100
LDA SUB      / Load subtrahend
CMA          / Complement
INC          / +1  →  two's complement negation
ADD MIN      / Add minuend
STA DIF      / Store result
HLT

MIN, DEC 83
SUB, DEC -23
DIF, HEX 0
END
```

Assemble it:

```sh
manoasm subtract.masm        # produces subtract.mano
```

Inspect the intermediate output if needed:

```sh
manoasm --dump-tokens --dump-symbols --dump-ast subtract.masm
```

See [docs/Assembler.md](docs/Assembler.md) for the full CLI reference and more examples.

---

## Documentation

### Architecture

| Document | Description |
|----------|-------------|
| [docs/Architecture.md](docs/Architecture.md) | Memory, bus, fetch-decode-execute cycle, I/O, and interrupts |
| [docs/Registers.md](docs/Registers.md) | CPU registers, I/O flags, and register-transfer notation |
| [docs/Instructions.md](docs/Instructions.md) | Full instruction set with opcodes and semantics |
| [docs/Assembly.md](docs/Assembly.md) | Assembly language syntax and formal grammar |

### Sub-projects

| Document | Description |
|----------|-------------|
| [docs/Assembler.md](docs/Assembler.md) | Assembler CLI, pipeline internals, and sample programs |

---

## AI Usage

AI tools were used during the development of this project as a supporting tool for:

- Brainstorming and discussing design decisions
- Asking technical questions
- Reviewing and reasoning about code
- Identifying potential bugs and implementation issues
- Refining and structuring documentation

The implementation and architecture of the project were primarily developed and written manually. AI-generated code was used minimally, and any such code was reviewed and integrated manually.

Some documentation may have been refined with AI assistance for clarity, structure, and presentation.

## Version

`v1.0.0`
