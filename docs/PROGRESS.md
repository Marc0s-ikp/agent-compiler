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
Pass count: default 285/285 before, 290/290 after (86 of 86 compiler_tests, 204 of 204 custom). `--ir` 18/285 before, 34/290 after. No previously passing test fails on either path (checked by the reviewer against the baseline).
Fixed: v3 step 2. Lowering code moved next to each `EmitRISC` (D23 a), partial `--emit-ir` dump on lowering failure (D23 c), large immediates verified with `ir_large_immediates`, and control flow in the IR: if/else, while, do-while, for, break/continue, `&&`, `||`, `?:`, switch. New custom tests: `ir_large_immediates`, `ir_loops`, `ir_short_circuit`, `ir_ternary`, `ir_switch`.
Blocked: nothing. Review notes are under "Nice to have from the v3 step 2 review" in docs/BLOCKERS.md. D24 is pending Marcus.
Step 3 (calls and ABI) is done on the `--ir` path: default 293/293, `--ir` 135/293. Remaining `--ir` failures by first blocker: "this type" 70 (arrays, floats, structs), global variable 43, `SizeofType` 11, `StringLiteral` 6, `Deref` 6, `AddressOf` 6, non-integer operand 4, `SizeofExpr` 4, `Index` 4, others 4. D25 is pending Marcus.
Next: v3 step 4, arrays, pointers, globals.

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
| custom (written by tester, D13) | 199 | 199 |

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
