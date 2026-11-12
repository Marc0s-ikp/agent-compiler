# Progress

## Features
- [x] 1. Integer expressions, locals, return
- [x] 2. Control flow
- [x] 3. Functions
- [x] 4. Arrays and pointers
- [x] 5. Globals, static, enum, typedef, switch
- [x] 6. char, strings, sizeof
- [x] 7. float / double
- [x] 8. structs, unions

## Latest session
Pass count: default 299/299 before, 304/304 after (86 of 86 compiler_tests, 218 of 218 custom). `--ir` 226/299 before, 256/304 after. No previously passing test fails on either path (checked per test against the baselines).
Fixed: v3 step 5, float and double on the IR path. Classifier F32/F64 rows in `RiscV32Abi` (fa registers, then integer registers, a7/stack split, stack; returns in fa0) with the interface unchanged; float constants as read-only pool entries; signed and unsigned int<->float/double and float<->double conversions; compares (NaN-correct `!=`) and the full arithmetic set. New custom tests `ir_float_mixed_args`, `ir_float_double_in_ints`, `ir_float_split_a7`, `ir_float_utod`, `ir_float_dtou`. `tests/abi_check` now generates float/double signatures that exhaust fa0-fa7 and split doubles; seeds 101-505 x 300 pass in all four caller/callee combinations with GCC -O0 and -O2. BLOCKERS: new "Required before AArch64" items (pointer width, `AddCase`).
Blocked: nothing. Review approved in round 1; notes under "Nice to have from the v3 step 5 review" in docs/BLOCKERS.md. All 48 remaining `--ir` failures are struct/union tests (step 6).
Next: v3 step 6, structs and unions.

## v3 migration (IR path)
| Step | Default | --ir |
|------|---------|------|
| baseline (master before step 1; `--ir` was not parsed) | 283/283 | 0/283 |
| 1a TargetInfo, SetTarget, Target() (old path) | 283/283 | 0/283 |
| 1b IR core, --ir/--emit-ir flags, checker, IRGen | 283/283 | 0/283 |
| 1c RISC-V backend skeleton (frames, prologue, ret, WriteData) | 283/283 | 0/283 |
| 1d integer operators, assignment, locals, return via EmitIR | 284/284 | 18/284 |
| 1e review fixes (narrow return guard, data items, checker, main) | 285/285 | 18/285 |
| 2a move lowering code into the per-node files (D23 a) | 285/285 | 18/285 |
| 2b partial `--emit-ir` dump when lowering throws (D23 c) | 285/285 | 18/285 |
| 2c large immediates verified in the backend, custom test `ir_large_immediates` (default 286/286, --ir 19/286) | 286/286 | 19/286 |
| 2d control flow: if/else, loops, break/continue, &&, ||, ?:, switch; custom tests `ir_loops`, `ir_short_circuit`, `ir_ternary`, `ir_switch` | 290/290 | 34/290 |
| 3a `?:`, `&&`, `||` through a frame object, checker enforces one definition per temp (D24) | 290/290 | 34/290 |
| 3b calls and ABI: IR `call`, parameters, outgoing area, classifier (D25), narrow returns; custom tests `ir_call_stack_args`, `ir_call_params`, `ir_recursion` | 293/293 | 135/293 |
| 4a neutral names, ptradd/ptrtoint/inttoptr, arrays, pointers, sizeof (custom test `ir_pointer_forms`) | 294/294 | 176/294 |
| 4b globals and static locals as data entries, string literals as entries; custom tests `ir_neg_index`, `ir_unsigned_index`, `ir_ptr_difference`, `ir_addr_of_elem`, `ir_global_ptr_elem` | 299/299 | 226/299 |
| 4c review fixes: `ir_addr_of_elem` checked against GCC-computed addresses, docs | 299/299 | 226/299 |
| 5a float and double: classifier rows, conversions, compares, float pool entries, backend | 299/299 | 251/299 |
| 5b custom tests `ir_float_mixed_args`, `ir_float_double_in_ints`, `ir_float_split_a7`, `ir_float_utod`, `ir_float_dtou`; float/double in `abi_check` | 304/304 | 256/304 |
| 5c review: `abi_check` exhausts fa registers and splits doubles, split-double docs corrected | 304/304 | 256/304 |

## Pass count by category
| Category | Passing | Total |
|----------|---------|-------|
| _example | 1 | 1 |
| array | 5 | 5 |
| control_flow | 12 | 12 |
| default | 4 | 4 |
| float | 7 | 7 |
| functions | 10 | 10 |
| integer | 12 | 12 |
| local_var | 6 | 6 |
| misc | 6 | 6 |
| pointer | 5 | 5 |
| programs | 3 | 3 |
| strings | 5 | 5 |
| struct | 5 | 5 |
| types | 5 | 5 |
| **Reference total** | **86** | **86** |
| custom (written by tester, D13) | 218 | 218 |

## History
| Date | Passing | Change |
|------|---------|--------|
| v1 | 110/160 | Previous attempt, different test set, code not available |(baseline) |
| 2026-11-08 | 44/106 | integer: compiler skeleton and integer expressions (0→44) |
| 2026-11-08 | 73/123 | control_flow: if, while, for, do-while, break and continue (44→73) |
| 2026-11-08 | 101/138 | functions: calls, prototypes and stack arguments (73→101) |
| 2026-11-08 | 127/155 | pointer: arrays, pointers and pointer arithmetic (101→127) |
| 2026-11-08 | 159/180 | feature 5: globals, static, enum, typedef, switch (127→159) |
| 2026-11-09 | 195/207 | feature 6: char, strings, sizeof, local and global initialisers (159→195) |
| 2026-11-09 | 219/224 | float: float and double, casts, floating ABI (195→219) |
| 2026-11-10 | 262/262 | feature 8: structs, unions, by-value struct ABI (219→262) |
| 2026-11-10 | 272/272 | cleanup: self-referencing initialisers, pointers in ?:, short (262→272) |
| 2026-11-10 | 283/283 | constant folding: typed 32-bit folding for initialisers, case, enum, array sizes (272→283) |
| 2026-11-11 | 285/285 | v3 step 1: TargetInfo, IR core, checker, backend skeleton, integer lowering (default 283→285, --ir 0→18) |
| 2026-11-11 | 290/290 | v3 step 2: control flow in the IR (default 285→290, --ir 18→34) |
| 2026-11-12 | 293/293 | v3 step 3: one definition per temp (D24), calls and ABI, classifier interface D25 (default 290→293, --ir 34→135) |
| 2026-11-12 | 294/294 | v3 step 4 stage A: neutral labels, ptradd/ptrtoint/inttoptr, arrays and pointers, sizeof (default 293→294, --ir 135→176) |
| 2026-11-12 | 299/299 | v3 step 4 stage B: globals, static locals and strings as data entries, five pointer tests (default 294→299, --ir 176→226) |
