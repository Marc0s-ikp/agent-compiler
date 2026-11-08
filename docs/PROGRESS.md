# Progress

## Latest session
Pass count: 0 at session start, 127/155 now (58 of 86 compiler_tests, 69 of 69 custom).
Fixed: features 1-4 merged (integer expressions and locals, control flow, function calls, arrays and pointers).
Blocked: one open item in docs/BLOCKERS.md (frame-size overflow at about 2 GB of locals, waived by Marcus). No test depends on it.
Remaining failures: 28 tests, all needing later features (globals, enum, typedef, switch, char, strings, sizeof, floats, structs).
Next: feature 5 (globals, static, enum, typedef, switch), waiting for Marcus's go-ahead.

## Pass count by category
| Category | Passing | Total |
|----------|---------|-------|
| _example | 1 | 1 |
| array | 4 | 5 |
| control_flow | 12 | 12 |
| default | 4 | 4 |
| float | 0 | 7 |
| functions | 10 | 10 |
| integer | 12 | 12 |
| local_var | 6 | 6 |
| misc | 0 | 6 |
| pointer | 5 | 5 |
| programs | 3 | 3 |
| strings | 0 | 5 |
| struct | 0 | 5 |
| types | 1 | 5 |
| **Reference total** | **58** | **86** |
| custom (written by tester, D13) | 69 | 69 |

## History
| Date | Passing | Change |
|------|---------|--------|
| v1 | 110/160 | Previous attempt, different test set, code not available |(baseline) |
| 2026-11-08 | 44/106 | integer: compiler skeleton and integer expressions (0→44) |
| 2026-11-08 | 73/123 | control_flow: if, while, for, do-while, break and continue (44→73) |
| 2026-11-08 | 101/138 | functions: calls, prototypes and stack arguments (73→101) |
| 2026-11-08 | 127/155 | pointer: arrays, pointers and pointer arithmetic (101→127) |
