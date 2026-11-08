# Progress

## Features
- [x] 1. Integer expressions, locals, return
- [x] 2. Control flow
- [x] 3. Functions
- [x] 4. Arrays and pointers
- [x] 5. Globals, static, enum, typedef, switch
- [ ] 6. char, strings, sizeof (in progress on feat/char-strings-sizeof)
- [ ] 7. float / double
- [ ] 8. structs, unions

## Latest session
Pass count: 159/180 before and after (65 of 86 compiler_tests, 94 of 94 custom). No compiler code changed this session. On feat/char-strings-sizeof the suite reads 159/205, because the 25 new tests fail until feature 6 exists.
Fixed: nothing new. Feature 5 is complete and merged on master at ef26bb9 (globals and extern, static, enum, typedef, switch), with no feature 5 work outstanding.
Blocked: no new blockers. The session stopped early on usage limits. The frame-size item stays open, and D15 and D16 await Marcus's acceptance.
Feature 6 is started but not implemented: feat/char-strings-sizeof holds only the WIP commit of 25 custom tests (checked against riscv gcc) and this update. Not reviewed, not merged, not pushed.
Next: implement feature 6 on that branch. In order: `char`/`signed` keywords, char literals, `sizeof` (with the const Resolve split from BLOCKERS), string literals and the data pool, then local and global initialisers. It should fix 9 reference tests (strings 5, types 4). Then review, merge and record it here.

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
