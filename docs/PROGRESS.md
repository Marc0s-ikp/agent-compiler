# Progress

## Latest session
Features 1 (integer expressions, locals, return, integer parameters) and 2 (control flow) merged.
Suite: 73/123 (36 of 86 compiler_tests, 37 of 37 custom).

## Pass count by category
| Category | Passing | Total |
|----------|---------|-------|
| _example | 1 | 1 |
| array | 0 | 5 |
| control_flow | 12 | 12 |
| default | 3 | 4 |
| float | 0 | 7 |
| functions | 0 | 10 |
| integer | 12 | 12 |
| local_var | 6 | 6 |
| misc | 0 | 6 |
| pointer | 0 | 5 |
| programs | 1 | 3 |
| strings | 0 | 5 |
| struct | 0 | 5 |
| types | 1 | 5 |
| **Reference total** | **36** | **86** |
| custom (written by tester, D13) | 37 | 37 |

## History
| Date | Passing | Change |
|------|---------|--------|
| v1 | 110/160 | Previous attempt, different test set, code not available |(baseline) |
| 2026-11-08 | 44/106 | integer: compiler skeleton and integer expressions (0→44) |
| 2026-11-08 | 73/123 | control_flow: if, while, for, do-while, break and continue (44→73) |
