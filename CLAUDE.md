# Agent Compiler (v2)

C90 subset → RISC-V (RV32IMFD, ilp32d) compiler, written in C++20 with flex/bison.
Built by Claude agents; Marcus makes design decisions and reviews.
Previous attempt (v1): 110/160 tests passing. Goal: beat it, then reach 160/160.

## Ground truth
- `./scripts/run_tests.sh` is the only measure of progress. Never claim a test passes without running it.
- Results are written to `results/latest.txt` (one line per test: PASS/FAIL/CRASH/TIMEOUT + category/name).
- `docs/PROGRESS.md` holds the current pass count per category. Update it after every merged change.

## Build
- `make build/c_compiler` builds the compiler.
- Usage: `build/c_compiler -S in.c -o out.s`
- Test one case: `./scripts/run_tests.sh tests/compiler_tests/<category>/<name>.c`

## Architecture (do not change without a DECISIONS.md entry)
- `src/lexer.flex`, `src/parser.y` → AST
- `include/ast_*.hpp` → one node class per construct, each with `EmitRISC(std::ostream&, Context&)`
- `include/context.hpp` → symbol tables, scopes, stack frame layout, type info, label generation
- Types are first-class from day one: every expression node can report its type and size.
  (v1 lost most of its failures to bolted-on types: pointers, arrays, floats, structs.)
- Follow the RISC-V ilp32d calling convention exactly. Callee-saved registers must be saved.

## Rules for every agent
1. One feature or fix per commit. Commit message: `<category>: <what changed> (N→M passing)`.
2. Run the full suite before committing. A change that drops ANY previously passing test is not committed — revert or fix.
3. Don't edit anything under `tests/compiler_tests/` — those are the reference tests.
4. New tests you write go in `tests/custom/` with a `_driver.c` in the same format.
5. If stuck on the same failure after 3 attempts, stop, write what you tried in `docs/BLOCKERS.md`, and move on.
6. Record any architectural choice in `docs/DECISIONS.md` (date, decision, reason, alternatives).
7. Keep functions small and code readable — Marcus must be able to explain every part.
8. Don't edit Dockerfile, Makefile or the original scripts in scripts/ (only run_tests.sh may change, and only by Marcus).

## Feature order (do in this order, don't skip ahead)
1. Integer expressions, locals, return
2. Control flow: if/else, while, for, do-while, break/continue
3. Functions: calls, params (>8 args via stack), recursion
4. Arrays and pointers, pointer arithmetic
5. Globals, static, enum, typedef, switch
6. char, strings, sizeof
7. float / double (FP registers + ABI)
8. structs, unions

## v3 migration (D21, accepted by Marcus 2026-11-11)
Goal: a linear IR between the AST and code generation, so a second ISA needs only a new backend.
Design in `docs/DECISIONS.md`, "D21 detail". The old `EmitRISC` path and the new `--ir` path coexist
until step 7.
- Merge rule for every step: the default (old) path stays at 283/283, and the `--ir` pass count never
  goes down. Commit message: `ir/<step>: <what changed> (default 283/283, --ir N→M)`.
- Record both counts in `docs/PROGRESS.md` after every merge.
- Measure the IR path with `COMPILER_FLAGS=--ir ./scripts/run_tests.sh`.
- `Target()` used before `SetTarget` must abort, and must never fall back to a default.
- One stack slot per IR temporary, never reused (reuse is a possible later optimisation, not v3).
- The IR checker runs on every `--ir` compile, in every build (never behind `#ifdef DEBUG`), so
  every `COMPILER_FLAGS=--ir` suite run checks the IR.

Do these in order, don't skip ahead:
1. Expressions and locals: `TargetInfo` replaces the size constants (on the old path first), IR core,
   `--emit-ir`, the IR checker (temps defined before use, operand types consistent), module data
   entries, backend frame layout, prologue/epilogue, integer ops, assignment, return
2. Control flow: if/else, loops, break/continue, `&&` `||` `?:`, switch
3. Calls and ABI: IR `call`, the per-target classifier interface, the RISC-V classifier (D5/D18/D19)
   moved behind it, >8 args, recursion
4. Arrays, pointers, globals: addresses, load/store, pointer scaling, global/static data entries
   from D15 `ConstValue`, enum, char, string entries, sizeof
5. float / double: FP IR types, conversions, float-constant entries, the classifier's floating rows
6. structs, unions: memcpy/memzero, struct args and returns, initialisers
7. Delete the old path: `--ir` becomes the default at 283/283, then remove `EmitRISC`, the flag and
   the dead `Context`/`emit` code, and update the Architecture section above
