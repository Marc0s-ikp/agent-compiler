# Progress

## Latest session
Features 1 (integer expressions, locals, return, integer parameters), 2 (control flow) and 3 (function calls) merged.
Suite: 101/138 (49 of 86 compiler_tests, 52 of 52 custom).

## Pass count by category
| Category | Passing | Total |
|----------|---------|-------|
| _example | 1 | 1 |
| array | 0 | 5 |
| control_flow | 12 | 12 |
| default | 4 | 4 |
| float | 0 | 7 |
| functions | 10 | 10 |
| integer | 12 | 12 |
| local_var | 6 | 6 |
| misc | 0 | 6 |
| pointer | 0 | 5 |
| programs | 3 | 3 |
| strings | 0 | 5 |
| struct | 0 | 5 |
| types | 1 | 5 |
| **Reference total** | **49** | **86** |
| custom (written by tester, D13) | 52 | 52 |

## History
| Date | Passing | Change |
|------|---------|--------|
| v1 | 110/160 | Previous attempt, different test set, code not available |(baseline) |
| 2026-11-08 | 44/106 | integer: compiler skeleton and integer expressions (0→44) |
| 2026-11-08 | 73/123 | control_flow: if, while, for, do-while, break and continue (44→73) |
| 2026-11-08 | 101/138 | functions: calls, prototypes and stack arguments (73→101) |
