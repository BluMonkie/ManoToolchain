# Emulator (`manoemu`)

`manoemu` loads a binary memory image (`.mano`) produced by `manoasm` and executes it as a Mano Basic Computer.

---

## Usage

```sh
manoemu [options] <input_file.mano>
```

### Options

| Option | Description |
|--------|-------------|
| `--help` | Print usage information |
| `--version` | Print version information |

### Quick Start

```sh
# Assemble and run a program
manoasm tests/assembler/subtract.masm
manoemu tests/assembler/subtract.mano

# One-liner (assemble then immediately run)
manoasm subtract.masm && manoemu subtract.mano
```

---

## Input Format

The emulator expects a flat **big-endian binary** file — exactly the format produced by `manoasm`:

- **Size:** up to 8192 bytes (4096 × 16-bit words)
- **Byte order:** big-endian, most significant byte first
- **Unused words:** default to `0x0000`

The file is loaded verbatim into the emulated 4096-word memory space starting at address `0x000`.

---

## Execution Pipeline

```
Binary (.mano)
      │
      ▼
  ┌──────────┐
  │  Loader  │  Reads the file into the emulated memory array
  └──────────┘
      │
      ▼
  ┌──────────┐
  │   I/O    │  Initialises the terminal for raw, unbuffered input
  │   Init   │
  └──────────┘
      │
      ▼
  ┌──────────┐
  │ Executor │  Fetch → Decode → Execute loop until HLT
  └──────────┘
```

Each cycle of the executor:

1. **Check for pending output** — if `FGO = 0`, write `OUTR` to the terminal and set `FGO ← 1`.
2. **Check for available input** — if `FGI = 0` and a byte is waiting on stdin, read it into `INPR` and set `FGI ← 1`.
3. **Check for a pending interrupt** — if `IEN = 1` and (`FGI = 1` or `FGO = 1`), perform the interrupt cycle.
4. **Fetch** — read the 16-bit word at `M[PC]` and increment `PC`.
5. **Decode** — classify the word as a memory-reference, register-reference, or I/O instruction.
6. **Execute** — carry out the operation.

---

## CPU State

At startup all CPU registers and flags are zero-initialised, except:

| State | Initial value | Reason |
|-------|:-------------:|--------|
| `FGO` | `1` | Output device is ready immediately |
| All others | `0` | Zero-initialised |

This matches the architectural initial state described in [IO.md](IO.md).

---

## I/O Model

The emulator models the Mano computer's asynchronous I/O using the host terminal:

### Input

- The terminal is placed in **raw mode** (`ICANON` and `ECHO` disabled) so characters are available immediately on keypress, without waiting for a newline.
- `is_input_available()` uses a zero-timeout `select()` call to check for a pending byte without blocking.
- When a byte is available, it is read into `INPR` and `FGI` is set to `1`.
- Characters are **not echoed automatically**. A program must execute `OUT` after `INP` to echo input.
- Terminal settings are restored on exit via `atexit`.

### Output

- When `FGO` goes to `0` (set by `OUT`), the emulator writes the byte in `OUTR` to stdout using `putchar` and immediately flushes.
- `FGO` is then set back to `1`, signalling that the output device is ready.

> **Platform note:** Two I/O backends are compiled conditionally. On POSIX systems (Linux, macOS), `io_posix.c` switches stdin into raw mode using `termios` so characters are delivered immediately without waiting for Enter. On Windows, `io_win32.c` uses `<conio.h>` — `_kbhit()` for non-blocking input polling, `_getch()` for unbuffered, non-echoing character reads, and `_putch()` for output. Both backends provide identical behaviour; the Windows version requires no terminal setup because `conio.h` already operates in raw mode.

---

## Interrupt Handling

When `IEN = 1` and at least one device flag (`FGI` or `FGO`) is set at the start of a cycle, the emulator performs the hardware interrupt sequence before fetching the next instruction:

| Step | Action |
|------|--------|
| 1 | `M[0x000] ← PC` — save return address |
| 2 | `PC ← 0x001` — jump to ISR |
| 3 | `IEN ← 0` — disable further interrupts |

The ISR at address `0x001` is responsible for servicing the device and re-enabling interrupts. See [IO.md](IO.md) for an ISR skeleton.

---

## Error Handling

| Condition | Behaviour |
|-----------|-----------|
| Invalid instruction word | Prints `Error: Invalid instruction <word> at <addr>` to stderr and exits with failure |
| Input file missing | Prints `ERROR: Missing source file. See manoemu --help.` to stderr and exits |
| Binary larger than 4096 words | Calls `abort()` |
| Unknown CLI option | Prints error to stderr and exits |

---

## Related

- [Assembler.md](Assembler.md) — Assembler CLI and output format
- [IO.md](IO.md) — I/O registers, flags, and interrupt-driven I/O
- [Instructions.md](Instructions.md) — Full instruction set with opcodes and semantics
- [Architecture.md](Architecture.md) — Fetch–decode–execute cycle and CPU architecture
