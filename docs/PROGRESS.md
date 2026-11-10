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
Pass count: 219/224 before, 262/262 after (86 of 86 compiler_tests, 176 of 176 custom). No test that passed before now fails.
Fixed: feature 8 (reviewer approved 2026-11-10): `struct` and `union` types, tags, member access, copy and assignment, initialisers, and by-value arguments and returns in the ilp32d ABI, including the hardware floating-point rules and hidden result pointers. Reference struct is 5/5. 38 new custom tests (36 from the tester, plus `struct_edge_abi` and `struct_big_frame`).
Blocked: no new blockers. Non-blocking items from the review are in BLOCKERS "Nice to have from the feature 8 review". D19 awaits Marcus's acceptance.
Remaining failures: none.
Next: all 8 features are done. Marcus decides what follows; the open items are in docs/BLOCKERS.md.

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
| custom (written by tester, D13) | 176 | 176 |

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
