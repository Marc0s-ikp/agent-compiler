# Progress

## Features
- [x] 1. Integer expressions, locals, return
- [x] 2. Control flow
- [x] 3. Functions
- [x] 4. Arrays and pointers
- [x] 5. Globals, static, enum, typedef, switch
- [x] 6. char, strings, sizeof
- [x] 7. float / double
- [ ] 8. structs, unions

## Latest session
Pass count: 195/207 before, 219/224 after (81 of 86 compiler_tests, 138 of 138 custom). No test that passed before now fails.
Fixed: feature 7 (reviewed 2026-11-09: approved, no blocking findings; 71 edge-case values matched GCC -O0; merged by Marcus's approval): `float` and `double` types, floating literals, arithmetic, comparisons and conditions, conversions, cast expressions (integer casts too), the floating ABI rows including the split double, and floating global and local initialisers. Reference float is 7/7. 17 new custom tests (14 float tests from the tester, plus `float_split_double`, `float_nan_truth`, `cast_integer`).
Blocked: no new blockers. The frame-size item stays open. Marcus accepted D17 and D18 on 2026-11-09.
Remaining failures: struct (5), all compiler crashes because the feature does not exist yet.
Next: feature 8, structs and unions. See BLOCKERS "Do first" item 5.

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
| struct | 0 | 5 |
| types | 5 | 5 |
| **Reference total** | **81** | **86** |
| custom (written by tester, D13) | 138 | 138 |

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
