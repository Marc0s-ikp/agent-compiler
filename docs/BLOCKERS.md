# Blockers
Root causes agents couldn't fix after 3 attempts. Marcus reviews these.

## 2026-11-08: feature 1 not approved after one round of changes

Branch `feat/integer-expressions`, at 2dc1bfd. Suite: 44/106 (24 reference, 20 custom), no regressions.
Not merged: the startup loop allows one round of changes, and the reviewer still says CHANGES NEEDED.

**Remaining must-fix:** `src/ast_function.cpp:53-55`. `FunctionDefinition::EmitRISC` keeps only the name
from `declarator_.Apply(...)`, then calls `return_spec_.Resolve(ctx)` again for `BeginFunction`. The
type the declarator produces is thrown away.

**Effect:** none today, because the Name declarator returns its base type unchanged. Once pointer
declarators exist (feature 4), `int *f()` would get return type `int`, and `char *f()` would truncate the
returned pointer to 8 bits.

**Proposed fix:** resolve once, `auto [name, type] = declarator_.Apply(...)`, then `BeginFunction(type)`.

**Tried so far:** first review found 3 must-fix items. All were addressed in 2dc1bfd, and this gap in the
declaration change is what the re-review found.

**Resolved 2026-11-08:** Marcus allowed one extra round. Fixed in 13928de and approved by the reviewer.

## 2026-11-08: feature 4 not approved after one round of changes

Branch `feat/arrays-pointers`, at 65e8372. Suite: 127/155 (58 reference, 69 custom), no regressions.
Not merged: the startup loop allows one round of changes, and the reviewer still says CHANGES NEEDED.
The same root cause (oversized arrays corrupting the frame) has now failed twice.

**Remaining must-fix:** `src/context.cpp:68`, with the same arithmetic in `FrameSize` at line 91.
`Declarator::Apply` now guarantees one array's size fits in an `int`, but `Allocate` computes
`used_ + type->Size()` unchecked.

**Effect:** only programs with about 2 GB of locals. Reproduced by the director:
`int a[536870911];` exits 0 with a UBSan overflow report, a 16-byte frame, and the array addressed at
`s0 + 2147483640`. Valid programs of realistic size are unaffected.

**Proposed fix:** in `Allocate`, throw "stack frame too large" before the addition, with a cap that
leaves room for alignment, outgoing bytes and the rounding in `FrameSize()`. For example,
`if (type->Size() > kMaxFrame - used_)`.

**Tried so far:** first review reported the bound wrapping. 65e8372 added `*n > INT_MAX / base->Size()`
in `Declarator::Apply`, which bounds one array but not the frame.

**Status 2026-11-08: OPEN.** Marcus waived reviewer approval for this item and feature 4 was merged as is.
The fix above has not been made.
