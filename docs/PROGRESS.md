# Progress

## Features
- [x] 1. Integer expressions, locals, return
- [x] 2. Control flow
- [x] 3. Functions
- [x] 4. Arrays and pointers
- [x] 5. Globals, static, enum, typedef, switch
- [x] 6. char, strings, sizeof
- [ ] 7. float / double
- [ ] 8. structs, unions

## Latest session
Pass count: 159/205 before, 195/207 after (74 of 86 compiler_tests, 121 of 121 custom). No test that passed before now fails.
Fixed: feature 6 is reviewed, approved and merged: `char` types, char and string literals, `sizeof`, and local and global initialisers, including `char *s = "hi"` and `char s[] = "hi"` at file scope. Reference strings and types are now 5/5 each.
Blocked: no new blockers. The frame-size item stays open. D15 and D16 are accepted, and D17 awaits Marcus's acceptance. The review's nice-to-have items are in BLOCKERS.
Remaining failures: float (7) and struct (5), all compiler crashes because those features do not exist yet.
Next: feature 7, float / double. Do BLOCKERS "Do first" item 4 and the `fa0` note for local initialisers as part of it.

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
| strings | 5 | 5 |
| struct | 0 | 5 |
| types | 5 | 5 |
| **Reference total** | **74** | **86** |
| custom (written by tester, D13) | 121 | 121 |

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
