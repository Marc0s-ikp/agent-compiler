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

## Architecture (do not change without a DECISIONS.md entry; design in `docs/DECISIONS.md`, "D21 detail")
Pipeline: source → `src/lexer.flex`, `src/parser.y` → AST → `EmitIR` (frontend) → `ir::Module` → IR checker
→ RISC-V backend → assembly. `build/c_compiler --emit-ir` prints the IR instead of assembly.
- **AST** (`include/ast_*.hpp`, `src/ast_*.cpp`): one node class per construct. Statements lower with
  `EmitIR(IRGen&)`; expressions with `EmitValue` (value) and `EmitIRAddress` (lvalue). Shared lowering
  helpers are in `ast_lower.*` (D24 a).
- **Frontend** knows C, not the machine: scopes, types, conversions, pointer scaling, constant folding,
  initialiser layout, evaluation order. It never names a register, instruction, frame offset or directive.
  `include/context.hpp` holds the frontend state: scopes and tags, globals and the string/float pools,
  loop/switch targets and case labels (D24 b), label and symbol names. `include/ir_gen.hpp` builds the IR.
- **IR** (`include/ir.hpp`): linear, typed instructions on temporaries and frame objects, plus module data
  entries. Every temp has exactly one defining instruction (D24 d). Struct values are `Ptr`s to frame
  objects; a struct-returning call writes its `result_object` (D26).
- **Checker** (`src/ir_check.cpp`) runs on every compile, in every build (never behind `#ifdef DEBUG`).
  Nothing is generated from a module that fails it.
- **Backend** (`src/backend_riscv.cpp`) knows the machine: frame layout, prologue/epilogue, instruction
  choice. One stack slot per IR temporary, never reused; nothing is held in a register between IR
  instructions (21.4). Frame addressed from `s0`, `sp` fixed after the prologue, aligned to 16 (21.6).
- **Target** (`include/target.hpp`): type sizes and alignment come from `Target()`. `Target()` or
  `Target().Abi()` used before `SetTarget` must abort, and must never fall back to a default.
- **ABI**: the ilp32d rules exist only in `RiscV32Abi` (`src/call_abi_riscv.cpp`), behind the `TargetAbi`
  interface (`include/call_abi.hpp`, D25), shared by caller and callee. Follow the ilp32d calling
  convention exactly; callee-saved registers the backend uses must be saved.
- Types are first-class: every expression node can report its type and size, and `GetType` emits nothing.
  (v1 lost most of its failures to bolted-on types: pointers, arrays, floats, structs.)

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

## v3 migration (D21, accepted by Marcus 2026-11-11): DONE 2026-11-12
Steps 1 to 7 are merged; the IR is the only code generation path and `EmitRISC`, the old `Context`
frame code and the `--ir` flag are gone (Step 1 to 7 notes in `docs/DECISIONS.md`). Its standing rules
are in the Architecture section above. A second ISA needs a new `TargetInfo`, a `TargetAbi` subclass and
a backend; the frontend and the IR do not change.
