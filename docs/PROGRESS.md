# Progress

## Latest session
Pass count: 127/155 at session start (58 of 86 compiler_tests), 159/180 now (65 of 86 compiler_tests, 94 of 94 custom, 25 custom added).
Fixed: feature 5 merged (globals and extern, static, enum, typedef, switch).
Blocked: no new blockers. The frame-size item stays open, and the reviewer's non-blocking items are under "Nice to have" in docs/BLOCKERS.md. D15 and D16 await Marcus's acceptance.
Remaining failures: 21 tests, all needing later features (float 7, strings 5, struct 5, types/sizeof 4).
Next: feature 6 (char, strings, sizeof), waiting for Marcus's go-ahead.

## Pass count by category
| Category | Passing | Total |
|----------|---------|-------|
| _example | 1 | 1 |
| array | 5 | 5 |
| control_flow | 12 | 12 |
| default | 4 | 4 |
| float | 0 | 7 |
| functions | 10 | 10 |
| integer | 12 | 12 |
| local_var | 6 | 6 |
| misc | 6 | 6 |
| pointer | 5 | 5 |
| programs | 3 | 3 |
| strings | 0 | 5 |
| struct | 0 | 5 |
| types | 1 | 5 |
| **Reference total** | **65** | **86** |
| custom (written by tester, D13) | 94 | 94 |

## History
| Date | Passing | Change |
|------|---------|--------|
| v1 | 110/160 | Previous attempt, different test set, code not available |(baseline) |
| 2026-11-08 | 44/106 | integer: compiler skeleton and integer expressions (0→44) |
| 2026-11-08 | 73/123 | control_flow: if, while, for, do-while, break and continue (44→73) |
| 2026-11-08 | 101/138 | functions: calls, prototypes and stack arguments (73→101) |
| 2026-11-08 | 127/155 | pointer: arrays, pointers and pointer arithmetic (101→127) |
| 2026-11-08 | 159/180 | feature 5: globals, static, enum, typedef, switch (127→159) |
