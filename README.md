# Agent Compiler

A C90-subset compiler written in C++20 with flex and bison, built by a team of Claude agents, with two
backends:

- **RISC-V**: RV32IMFD, the ilp32d calling convention (the coursework target).
- **AArch64**: AAPCS64, LP64 (`int` 4 bytes, `long` and pointers 8, plain `char` unsigned).

Both backends sit behind one typed intermediate representation, so the frontend (the part that knows C) is
shared and never names a register or an instruction. I set up the agents, decided the workflow, reviewed and
approved every design decision, and stepped in when they got stuck.

## Results

| Test set | riscv32 | aarch64 |
|----------|---------|---------|
| Reference tests (Imperial LangProc public suite) | **86 / 86** | **86 / 86** |
| Custom tests, both targets (`tests/custom/`) | **249 / 249** | **249 / 249** |
| Custom tests, one target only (`tests/custom_<target>/`) | 0 / 0 | **1 / 1** |
| **Total** | **335 / 335** | **336 / 336** |

The calling convention is checked separately against GCC, on both targets (see "How the ABI is verified"):

| ABI check (seeds 606 to 1010, 500 random signatures each) | riscv32 | aarch64 |
|-----------------------------------------------------------|---------|---------|
| Signatures, each run in 7 builds (ours and GCC -O0/-O2 as caller and callee) | 2,500 | 2,500 |
| Failures | **0** | **0** |
| ABI cases hit (per-case table in `docs/ABI_RESULTS.md`) | **all 17** | **all 58** |
| Model checked against GCC's own caller (`probe_aapcs64.py`) | n/a | **2,500 agree** |

No merge ever broke a test that already passed: every commit message carries the pass count before and after,
on both targets. The first attempt at this project (v1) reached 110/160 before it was abandoned; its
post-mortem is in `docs/V1_POSTMORTEM.md`.

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
- One feature per branch, one change per commit. A change that makes any passing test fail is not merged.
- The reference tests are read-only.
- Every architectural choice goes in `docs/DECISIONS.md` and waits for my approval.
- Anything still failing after three attempts, and every review finding that did not block a merge, goes to
  `docs/BLOCKERS.md` for me to decide.

The project went through three stages:

1. **A direct RISC-V compiler** (v2). Before any code was written, the director proposed a design and
   stopped for review; I added five requirements to it (D9 to D13). Features were then built in a fixed
   order, one per session: integer expressions, control flow, functions, arrays and pointers, globals and
   `switch`, `char` and strings, floating point, structs and unions. It passed all 283 tests at the time.
2. **The IR migration** (v3, D21). Code generation moved behind a typed IR in seven steps, with the old and
   new paths both running the full suite at every step until the old one was deleted. The point was the
   next stage: a second instruction set should need a backend and an ABI, not a second compiler.
3. **The AArch64 backend** (D27). Six steps (A1 to A6: expressions, control flow, calls, pointers and
   globals, floats, records), each measured with `TARGET=aarch64`. The aarch64 count only ever went up and
   riscv32 never dropped. The IR did not change; the frontend changes were the ones listed in advance
   (literal types, `size_t`, 64-bit constant folding).

The full history is in `docs/PROGRESS.md`, every design decision with its alternatives is in
`docs/DECISIONS.md`, and the review findings are in `docs/BLOCKERS.md`.

## Design

```
source --flex/bison--> AST --EmitIR (frontend)--> ir::Module --CheckIR--> RISC-V backend  --> assembly
                                                                     \--> AArch64 backend --> assembly
                        Target(): sizes and alignment       Target().Abi(): where each argument goes
```

**The frontend knows C, not the machine.** Each AST node lowers itself: statements with `EmitIR`,
expressions with `EmitValue` (the value) and `EmitIRAddress` (the lvalue, used by assignment, `&`, `++`,
indexing and members). Scopes, types, conversions, pointer scaling, constant folding, initialiser layout and
evaluation order are all decided here. Every expression reports its type without emitting anything (D3), so
`sizeof` has no side effects; v1 lost most of its failures to types bolted on later.

**The IR (D21)** is a linear list of typed instructions on temporaries and frame objects, plus module data
(globals, strings, the float pool). Types are machine-level (`i8` to `i64`, `f32`, `f64`, `ptr`) and
signedness belongs to the operation (`div`/`divu`, `shr`/`sar`). Every temporary has exactly one defining
instruction. `build/c_compiler --emit-ir` prints it. The same function on the two targets:

```
int scale(int *p, int i) { return p[i] * 3; }

riscv32                              aarch64
  frame $p.0 size 4 align 4            frame $p.0 size 8 align 8
  %3 = load i32 %2                     %3 = load i32 %2
  %4 = mul i32 %3, 4                   %4 = sext i32 %3 to i64
  %5 = ptradd ptr %1, %4               %5 = mul i64 %4, 4
                                       %6 = ptradd ptr %1, %5
```

The frontend scaled the index to pointer width because `Target()` said pointers are 8 bytes; neither
backend had to know C's rules for that.

**The checker** (`CheckIR`) runs on every compile, in every build, never behind `#ifdef DEBUG`: every temp
defined before use, operand types consistent, labels defined once, symbols resolved. No assembly is generated
from a module that fails it.

**Struct values are addresses (D26).** A struct or union is not an IR type. Its value is the `ptr` of a frame
object that holds it, and a call that returns one writes into the call's `result_object`. Copying is an IR
`memcpy`. Only at a call boundary is a record split into the pieces the ABI wants.

**One ABI classifier per target, shared by caller and callee (D5, D25).** `TargetAbi::Classify(params, ret)`
returns, for each parameter and the result, a list of pieces: an offset in the value, a byte count, the C
signedness, and an integer register, a floating register or a stack offset. The caller places pieces and the
callee reads them with the same answer, so the two sides cannot disagree. All of ilp32d lives in
`RiscV32Abi` (doubles split between `a7` and the stack, the floating-struct convention, by reference, the
hidden result pointer); all of AAPCS64 in `AArch64Abi` (homogeneous floating aggregates in `v` registers,
records up to 16 bytes in `x` registers, larger ones by reference, the result through `x8`).

**The backends know the machine.** Frame layout, prologue and epilogue, instruction choice. Each IR temporary
gets its own stack slot, and nothing is held in a register between IR instructions, so a nested call cannot
overwrite a live value and no callee-saved register needs saving beyond the frame pointer. The frame is
addressed from `s0` (RISC-V) or `x29` (AArch64), with `sp` fixed and 16-byte aligned after the prologue.
Offsets and constants too large for an instruction go through a scratch register.

**Where the standard is silent, GCC decides (D28).** AAPCS64 does not say how a union's members count toward
a floating aggregate. Interoperability with GCC-compiled code is the requirement, so GCC's behaviour is the
rule (the largest member counts), and it is confirmed by the probe below rather than read off its assembly.

## How the ABI is verified

The reference tests call across the ABI boundary only in the shapes their authors thought of. Two tools in
`tests/abi_check/` cover the rest.

**`abi_check`** (`gen_abi.py`, `run_abi.sh`) generates random signatures: 0 to 20 parameters drawn from the
integer types, pointers, `float`, `double` and a table of structs and unions, plus shaped draws that fill
the argument registers first (so the stack, register splits and "registers used up" cases come up often). It
writes a callee that checks every parameter and returns a value derived from them, and a caller that checks
the result. Caller and callee are each built by this compiler and by GCC at `-O0` and `-O2`, and all four
pairings are linked and run (seven builds per seed, since ours-to-ours is shared). `-O2` is mandatory:
GCC at `-O0` re-extends narrow values and hides missing extensions.

The generator contains its own small model of each calling convention, written from the standard and
independent of the compiler's classifier (the AAPCS64 one is the standard's rule list, `c1` to `c17`). It is
used only to count, for every run, how many signatures hit each ABI case: an argument in the last register,
a record split across registers and stack, an HFA that no longer fits, a result through `x8`, and so on. A
case with zero hits is untested and is reported as missing.

**The probe** (`probe_aapcs64.py`) checks the model itself: GCC agreeing with GCC proves nothing about the
model. A hand-written assembly callee records `x0`-`x8`, `d0`-`d7` and 96 stack words, and returns known
byte patterns. GCC's `-O2` caller calls it through a cast to each generated signature with arguments of
known bytes, and the script compares where every byte landed with what the model predicted. Removing one
rule from the model (C.3) makes it disagree on 15 of 300 signatures.

## Supported C

`int`, `char` (unsigned on both targets), `short`, `unsigned`, `float` and `double`. Pointers,
multi-dimensional arrays, structs, unions, enums, typedefs, `static` and `extern`. All control flow,
including `switch`. Recursion and more than 8 arguments, struct arguments and returns by value, string
literals, and brace initialisers, both local and global. Integer literals take their C90 type from the
target's sizes, so on AArch64 `0xFFFFFFFFFFFFFFFF` is an 8-byte `unsigned long` and folds the same as it runs.

## Known limits

These are listed in full in `docs/BLOCKERS.md`.

- **Not implemented:** `long` as a keyword (literals can still be `long`), bit-fields, function pointers,
  variadic functions, `goto`.
- **Specifier order:** type specifiers are accepted in one order only (`unsigned short`, not `short unsigned`).
- **Constant contexts:** `&&`, `||`, `?:` and casts of floating values are not accepted where a constant
  is required, such as case labels and array sizes.

## Build and run

You need Docker. The image contains both toolchains: RISC-V GCC with spike and pk, and
`aarch64-linux-gnu-gcc` with `qemu-aarch64`.

```bash
docker build -t agent-compiler .
./scripts/run_tests.sh                     # riscv32: builds, runs every test inside Docker, prints N/M passing
TARGET=aarch64 ./scripts/run_tests.sh      # the same tests on AArch64
TARGET=aarch64 ./scripts/run_tests.sh tests/custom/a6_hfa4_double.c   # one test
```

Results go to `results/latest.txt`, one line per test. A test is `<name>.c` (compiled by this compiler) and
`<name>_driver.c` (compiled by GCC, returns 0 on success). An optional `<name>.gccflags` holds extra GCC
flags for the driver, such as `-std=gnu90`. Tests in `tests/custom_riscv32/` and `tests/custom_aarch64/` run
only under that target.

To compile one file:

```bash
docker run --rm --user "$(id -u):$(id -g)" -v "$PWD":/w -w /w agent-compiler -c '
  make build/c_compiler &&
  build/c_compiler --target aarch64 -S input.c -o out.s &&      # or --target riscv32 (the default)
  aarch64-linux-gnu-gcc -static -o prog out.s driver.c &&
  qemu-aarch64 prog'
```

For RISC-V, link with `riscv64-unknown-elf-gcc -march=rv32imfd -mabi=ilp32d` and run with `spike pk prog`.
Add `--emit-ir` to print the IR instead of assembly.

The ABI check (output in `tests/abi_check/out/`, delete it afterwards):

```bash
docker run --rm --entrypoint "" --user "$(id -u):$(id -g)" -v "$PWD":/w -w /w agent-compiler bash -c '
  make build/c_compiler &&
  for s in 606 707 808 909 1010; do TARGET=aarch64 tests/abi_check/run_abi.sh $s 500 -O0 -O2; done &&
  mkdir -p tests/abi_check/out && python3 -B tests/abi_check/probe_aapcs64.py 606 500 tests/abi_check/out/probe'
```

Leave out `TARGET=aarch64` for riscv32. Each run prints the per-case hit table and ends with `every case hit`
or the list of missing cases.

## Layout

```
CLAUDE.md                 rules and architecture every agent reads
STARTUP_PROMPT.md         the director's loop
.claude/agents/           implementer, tester, reviewer definitions
src/, include/            the compiler
  ast_*                     AST nodes and their lowering (frontend)
  ir.hpp, ir_gen.hpp        the IR and its builder; ir_check.cpp is the checker
  target.hpp                type sizes and alignment per target
  call_abi*                 TargetAbi, RiscV32Abi, AArch64Abi
  backend_riscv.cpp         RISC-V backend
  backend_aarch64.cpp       AArch64 backend
tests/compiler_tests/     reference tests (read-only)
tests/custom/             tests written by the agents, run on both targets
tests/custom_<target>/    tests for one target only
tests/abi_check/          random ABI check against GCC, and the AAPCS64 probe
docs/                     DECISIONS, PROGRESS, BLOCKERS, ABI_RESULTS, V1_POSTMORTEM
```
