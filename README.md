# Agent Compiler

A C90-subset to RISC-V (RV32IMFD, ilp32d) compiler written in C++20 with flex and bison, built by a team
of Claude agents. I set up the agents, decided the workflow, reviewed and approved every design decision,
and stepped in when they got stuck.

## Results

| Test set | Passing |
|----------|---------|
| Reference tests (Imperial LangProc public suite) | **86 / 86** |
| Custom tests written by the tester agent | **197 / 197** |
| **Total** | **283 / 283** |

These results were checked with the coursework's original test runner (`scripts/test.py`) as well as the
project's own runner, and no feature merge ever broke a test that already passed. For struct and float
arguments, the reviewer also tested random function signatures in all four combinations of this compiler
and GCC as caller and callee, and the results matched in every combination.

## How it was built

Four roles run in Claude Code:

| Agent | Model | Job |
|-------|-------|-----|
| Director | Opus | Plans, delegates, merges. Never writes compiler code |
| Implementer | Sonnet | One feature or fix per branch |
| Tester | Sonnet | Runs the suite, groups failures by root cause, writes reproducers. Cannot edit source |
| Reviewer | Opus | Reviews each branch for hacks, ABI errors and design problems before merge |

Rules every agent follows (`CLAUDE.md`):

- The test runner is the only measure of progress.
- One feature per branch. A change that makes any passing test fail is not merged.
- The reference tests are read-only.
- Every architectural choice goes in `docs/DECISIONS.md` and waits for my approval.
- Anything still failing after the allowed review rounds goes to `docs/BLOCKERS.md` for me to decide.

Before any code was written, the director proposed a design and stopped for review. I added five
requirements to it (D9–D13): implicit function declarations, arrays never being loaded as values,
constant global initialisers, 16-byte frame alignment, and custom tests for features the reference suite
doesn't cover. After that, features were built in a fixed order, one per session:

1. Integer expressions and locals
2. Control flow
3. Functions and the calling convention
4. Arrays and pointers
5. Globals, enum, typedef, switch
6. char, strings, sizeof
7. float and double
8. structs and unions

Two cleanup sessions followed: one for valid C the compiler wrongly rejected plus `short`, and one for
compile-time constant folding.

The full history is in `docs/PROGRESS.md`, every design decision with its alternatives is in
`docs/DECISIONS.md`, and the review findings are in `docs/BLOCKERS.md`.

## Design in brief

- **One result register (D1).** Every expression evaluates into `a0` (or `fa0` for floating point), and
  intermediate values live in stack slots. No value is kept in a register while a sub-expression runs, so
  a nested call cannot overwrite anything. `s1`–`s11` are never used, so the callee-saved rule is met by
  construction.
- **Frame laid out after the body (D2, D12).** Each function body is generated into a buffer first, so
  the prologue is written once the frame size is known. The frame is addressed from `s0`, `sp` stays
  16-byte aligned, and large offsets go through a scratch register.
- **Types first (D3, D10).** Every expression reports its type without generating code, so `sizeof`
  never has side effects. Arrays always evaluate to their address.
- **Value and address paths (D4).** Every expression has a value path (`EmitRISC`) and an address path
  (`EmitAddress`). Assignment, `&`, `++`, indexing and struct members all go through the address path.
- **One ABI classifier (D5, D18, D19).** Caller and callee share one routine that decides where each
  argument goes. It covers the full ilp32d convention: doubles split across registers and stack, the
  special case for small structs with float fields, and large structs passed by reference or returned
  through a hidden pointer.
- **One conversion routine (D6, D18).** All implicit and explicit conversions share one routine, with a
  compile-time equivalent so constant folding matches run-time behaviour (D20).
- **Correct typedef parsing (D7, D16).** A scoped typedef table, read by the lexer, makes C's grammar
  unambiguous.

## Supported C

`int`, `char` (unsigned on this target), `short`, `unsigned`, `float` and `double`. Pointers,
multi-dimensional arrays, structs, unions, enums, typedefs, `static` and `extern`. All control flow,
including `switch`. Recursion and more than 8 arguments, struct arguments and returns by value, string
literals, and brace initialisers, both local and global.

## Known limits

These are listed in full in `docs/BLOCKERS.md`.

- **Not implemented:** `long`, bit-fields, function pointers, variadic functions, `goto`.
- **Specifier order:** type specifiers are accepted in one order only (`unsigned short`, not `short unsigned`).
- **Constant contexts:** `&&`, `||`, `?:` and casts of floating values are not accepted where a constant
  is required, such as case labels and array sizes.
- **Frame size:** a function with about 2 GB of locals overflows the frame size calculation without an
  error.

## Build and run

You need Docker. The image contains the RISC-V GCC toolchain, spike and pk.

```bash
docker build -t agent-compiler .
./scripts/run_tests.sh                     # builds, runs every test inside Docker, prints N/M passing
```

To compile one file:

```bash
docker run --rm --user "$(id -u):$(id -g)" -v "$PWD":/w -w /w agent-compiler -c '
  make build/c_compiler &&
  build/c_compiler -S input.c -o out.s &&
  riscv64-unknown-elf-gcc -march=rv32imfd -mabi=ilp32d -o prog out.s driver.c &&
  spike pk prog'
```

## Layout

```
CLAUDE.md               rules and architecture every agent reads
STARTUP_PROMPT.md       the director's loop
.claude/agents/         implementer, tester, reviewer definitions
.claude/settings.json   destructive git commands denied to agents
src/, include/          the compiler
tests/compiler_tests/   reference tests (read-only)
tests/custom/           tests written by the tester agent
docs/                   DECISIONS, PROGRESS, BLOCKERS
```
