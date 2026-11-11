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

## Nice to have

From the feature 5 review (2026-11-08). None of these blocked the merge. Item 3 was fixed in feature 6 and item 4 in the cleanup session. The others are still open.

1. DONE (constant folding, D20). `src/ast_operators.cpp:121`: `FoldBinary` folds in untyped 64-bit `long`, so `unsigned a = ~0u >> 1;` and
   `case (0u-2)/2:` compile to the wrong value. Runtime evaluation is correct. Fix: fold using `GetType` and wrap to 32 bits.
2. `src/ast_function.cpp:118`: `static int f(int); int f(int x){...}` still emits `.globl f`. The definition should
   inherit internal linkage from the earlier declaration.
3. DONE (feature 6). `src/global_init.cpp:91-93`: `Word` silently emits nothing for sizes other than 1, 2 or 4. Unreachable today, but
   `double` would shift the data after it. Add a throw.
4. DONE (feature 8 cleanup). `src/ast_function.cpp:76-77`: the initialiser is built before the name is declared, so `int **q = &q[0];` fails
   with "undeclared identifier". Feature 8's `struct n a = {&a}` would hit the same error.
5. `src/ast_pointer.cpp`: `Deref` has no constant-address override, so `int *p = *m;` and `&*a` are rejected.
6. `LogicalAnd`, `LogicalOr` and `Ternary` have no `EvalConst` (`BinaryCompare` got `EvalConstValue` in feature 7, so global initialisers can use it, but `enum { A = 1 < 2 }` still fails), so `enum { A = 1 < 2 }` is rejected.
7. `src/ast_function.cpp:79`: this size check looks unreachable, because the merge step already rejects mismatches.

Unsupported and deferred: local `extern` (`src/parser.y:356`), local function prototypes
(`src/ast_stmt.cpp:41-44`).

## Do first in later features

From the same review. Items 1-3 were done in feature 6 (D17). Local brace initialisers and local
`char s[] = "abc"` are done too.

1. DONE (feature 6). `include/ast_declaration.hpp:42`: `Resolve` mutates `Context` for enum bodies, but `sizeof(type)` and casts call
   it from `GetType`/`EvalConst`, which take `const Context&` (D3). Split it into a declaring resolve and a const
   lookup rather than using `const_cast`.
2. DONE (feature 6). `include/ast_node.hpp:42`, `src/global_init.cpp:17`: `char *g = "x"` needs `AddString`, but the initialiser walk
   has only a `const Context&`.
3. DONE (feature 6). `src/global_init.cpp:107` and `:42-52`: strings need a case at top level and in `Object`, for `char s[]`, `{"ab"}`
   and `char a[2][4] = {"ab","cd"}`.
4. DONE (feature 7, D18). D15 understated the float work. `ConstValue` needs a floating field, `FloatLiteral`, `UnaryOp`, `BinaryArith` and
   `Cast` need overrides, and `Scalar` must convert to the target type (`float f = 1;`, single-precision `0.1`).
5. DONE (feature 8, D19). Feature 8: struct member rules must not call `EndDeclaration()` (`src/parser.y:17`), member declarators must not
   go through `declarator_head`, `.`/`->` must accept `T_TYPE_NAME`, and Records must compare by identity, because
   structural `Equals` would recurse forever on `struct node { struct node *next; }`.

## Known limits left by feature 6

- Casts exist (feature 7). Type names accept pointer, array and parenthesised abstract declarators, but not function
  declarators (`sizeof(int (*)(int))`, `(int (*)(int))p`).
- An `enum { ... }` body inside a parameter list or a type name is rejected ("enum definition not allowed here"),
  because those paths only look types up (D17). Enums declared as ordinary declarations are unaffected.
- `short` is now a keyword (cleanup session): `short`, `short int`, `signed short [int]`, `unsigned short [int]`.
- A local initialiser zeroes the whole array before storing, even when the list covers it. Correct, slightly wasteful.

## Nice to have from the feature 6 review

From the feature 6 review (2026-11-09). None of these blocked the merge. Item 2 was fixed in the cleanup session, and the others are still open.

1. `src/ast_operators.cpp:337`: `Comma::GetType` returns `rhs_->GetType`, so the array does not decay.
   `int a[3]; sizeof(0, a)` gives 12, gcc gives 4. Fix: return `rhs_->ValueType(ctx)`.
2. DONE (cleanup). `src/ast_call.cpp:74`: narrow stack arguments are written with `sb`, but the psABI widens them to the full 4-byte
   slot. No visible effect today, because gcc's callee reads with `lbu`/`lb`. Fix: store the extended register with `sw`.
3. `src/parser.y:332-354`: unnamed array parameters in prototypes are a syntax error, as in `int g(char [], int []);`.
   Not new in feature 6. Fix: try `type_spec abstract_declarator` in `param_decl` and check for conflicts.
4. `src/lexer.flex:66`: a backslash-newline inside a string is a lexer error. Fix: strip continuations before decoding.
5. `src/escapes.cpp:22`: unknown escapes that gcc accepts (`"\e"`, `"\8"`) are rejected. From reading the code,
   out-of-range `"\400"` and `"\x141"` wrap to 8 bits (lines 32 and 47); this was not compared with gcc.
6. Duplication: the pointer-wrap loop appears four times in `src/parser.y` (236, 287, 295, 340) and should be one helper.
   The D8 char type `Type::Integer(1, false)` is spelled out in `src/ast_declaration.cpp:49-50`, `src/ast_literal.cpp:42`
   and `src/local_init.cpp:19`, and should be a `Type::Char()` helper.
7. `src/global_init.cpp:22`: the comment mentions a `size` parameter that does not exist.

Deliberate: `src/init_layout.cpp:36` rejects `char s[2]="abc"` and line 94 rejects excess elements, where gcc only warns.

For feature 8: DONE (D19). `src/local_init.cpp` now zeroes structs and unions as well as arrays.

## Known limits left by feature 7

- `long double` and the `L` suffix on a floating literal are rejected. Hex floating literals do not lex.
- `LogicalAnd`, `LogicalOr` and `Ternary` have no `EvalConstValue`, so `double d = 1 ? 2.0 : 3.0;` at file scope is rejected.
- A double split across a7 and the stack is not tested against GCC: GCC's own callee reads it with a misaligned `fld`
  that gives a wrong value under spike pk. `tests/custom/float_split_double.c` checks our caller against our callee.
- Constant folding of an out-of-range floating value (`int x = 1e20;`) is rejected as "not a constant" instead of
  saturating, and a literal beyond double's range becomes infinity without a warning.
- Nice to have (feature 7 review): `Cast::EvalConst` only folds integer operands, so a cast of a floating constant is
  rejected where an integer constant is required (`int a[(int)2.5];`, `enum {A=(int)2.5};`, `case (int)1.5:`). GCC accepts all three.

## Known limits left by feature 8

- A `struct` or `union` body is rejected inside a type name or parameter list (`sizeof(struct { int a; })`, `int f(struct { int a; } s)`), as enum bodies are (D17). A tag that is not yet declared in a parameter list (`int f(struct Unknown *p);`) is an "unknown tag" error where C declares it in the prototype's scope. Name the struct in a declaration first.
- No bit-fields, no anonymous struct or union members (C11), no function-pointer members, no flexible array members.
- Struct values cannot be compared (`==`), used in a condition, cast, or have `&&`/`||` applied; each is a compile error as in C.
- Every struct-returning call site keeps its own result slot until the function ends (D19 c), so a function with many such calls has a larger frame than needed.
- A by-reference struct parameter is copied twice (the caller's copy, then the callee's home slot).
- Constant addresses (`&global.member`) work for `.`, not `->`.
- `tests/custom/struct_fp_split_double` checks our caller against our callee only (GCC's callee misreads a split double under spike pk), as for `float_split_double`.

## Nice to have from the feature 8 review

From the feature 8 review (2026-11-10). None of these blocked the merge. Items 1, 2 and 5 were fixed in the cleanup session, and the others are still open.

1. DONE (cleanup). `src/ast_function.cpp:116`: a global whose initialiser takes its own address fails. `struct L { struct L *n; int v; }; struct L a = {&a, 3};` gives "undeclared identifier 'a'", while GCC compiles it and the local form works. Same root as item 4 of the feature 5 review. `src/ast_stmt.cpp:30` has the same order for static locals (from reading the code). Fix: declare the name before building the initialiser, handling unsized arrays.
2. DONE (cleanup). `src/ast_operators.cpp:467-468`: `q = c ? p : (void*)0;` with `struct S *p` is rejected as "incompatible pointers in ?:", while GCC accepts it. Fixed with the C rule: a null pointer constant takes the other operand's type, and a non-constant `void *` with an object pointer gives `void *`.
3. `src/ast_member.cpp:45-49`: `Member::EmitAddress` accepts a non-lvalue base, so `f().a = 3;` and `(c ? a : b).a = 3;` compile where GCC rejects them. Only invalid programs are affected.
4. `src/ast_function.cpp:78`, `src/ast_call.cpp:116`: an incomplete by-value parameter or return type gets a size-0 slot instead of an error (`struct S; int f(struct S x) { return 0; }`). Only invalid programs are affected.
5. DONE (cleanup). `src/emit.cpp:198-208`: `EmitLoadBytes` ignores `is_signed` for 2 and 3 bytes, although `include/emit.hpp` says it sign-extends. Unreachable while `short` is not a keyword.
6. Duplicated helpers: `RoundUp` in `src/context.cpp:12` and `src/abi.cpp:49`, `RoundUpTo` in `src/type.cpp:54`. The integer register name helper is in `src/record_abi.cpp:7` and, as `IntArgReg`, `src/ast_call.cpp:25`.
7. Clarity: `Cursor::TakeWordPair` (`src/abi.cpp:102`) is a one-line alias for `TakeDoubleAsInts`; `Flatten` rebuilds a scalar `Type` to get a `TypePtr` (`src/abi.cpp:28-30`); the comment at `include/type.hpp:6-8` is stale.
8. Simpler ownership option for records: `Type` holds a non-owning `RecordData*` and `Context` owns `vector<unique_ptr<RecordData>>`, which removes `ReleaseRecord`. `Type::Record` would then need the owner passed in. The reviewer considered the current shape sound.

## Nice to have from the cleanup review

From the cleanup review (2026-11-10). None of these blocked the merge, and none is fixed yet.

1. `src/ast_cast.cpp:35-36`: `Cast::IsNullPointerConstant` accepts a `void *` operand, so `(void*)(void*)0` counts as a null pointer constant. `sizeof(*(c ? (void*)(void*)0 : ip))` gives 4, GCC gives 1. Fix: require the operand to have integer type.
2. `src/emit.cpp:122`: a pointer converted to a narrow integer is not truncated, because `EmitNarrow` only runs for integer sources. `int a[10]; (int)(char)&a[0] < 256` gives 0, GCC gives 1. The same applies to `(unsigned short)p`. Fix: narrow whenever the target is an integer.
3. `src/parser.y:167-183`: type specifiers are accepted in one order only. `short unsigned`, `int short`, `short signed` and `char unsigned` are syntax errors, GCC accepts them.
4. `long` and `unsigned long` are not keywords (`long x;` is a syntax error).
5. `src/ast_operators.cpp:482-483`: `?:` rejects `void` operands, so `c ? g() : h();` with void functions gives "scalar operand required". GCC accepts it.
6. `src/context.cpp:172-179`: `int a[3]; int a[] = {1,2};` is rejected as a conflicting declaration. GCC accepts it and keeps `int[3]`.
7. `src/ast_operators.cpp:488`: `Ternary::GetType` does not reject a floating operand with a pointer, so `sizeof(*(c ? 0.0 : ip))` compiles to 4 where GCC errors. Only invalid programs are affected.
8. `extern int x; static int x = 3;` compiles, GCC rejects it. `DeclareGlobal` does not check for a linkage change. Only invalid programs are affected.
9. DONE (constant folding, D20). Constant folding still differs from GCC on unsigned division and `int` overflow: this is item 1 of the feature 5 review. The cleanup fuzz hit it 16 times in about 8000 output lines (12 reduced to that cause, 4 `case` label misses of the same kind not reduced).
10. `src/ast_node.cpp`: one stray extra blank line after `IsNullPointerConstant`. `CompleteStaticLocal` and `InitGlobal` are two entry points for the same incomplete-to-complete step and could be unified.

## Nice to have from the constant folding review

From the constant folding review (2026-11-10, approved). None of these blocked the merge, and none is fixed yet. Line numbers are at the merge.

1. `src/ast_operators.cpp:335`: the `IsPointer` test is dead, because `CompareType` returns `UInt` for pointers. The guards at lines 221 and 443 are also unreachable, since non-null `EvalConst` operands are always integers. Remove them or say why they stay.
2. `src/ast_operators.cpp:245`: the `UInt` versus `Int` choice for a pointer offset needs a comment. A label offset stays signed so `a - 1` prints `.word a-4`, while an absolute address is unsigned. `(char *)-1` still has offset -1, so the canonical form of a pointer constant is undefined. Consumers wrap, so this is harmless.
3. Duplication: `IsShift(op_) ? Promote(...) : result` appears three times (`src/ast_operators.cpp:222, 254, 289`). `BinaryCompare::EvalConst` and `EvalConstValue` could share a wrap-then-compare helper. `src/context.cpp:272` repeats the int wrap by hand.
4. `tests/custom/const_fold_case_labels.c:20`: `case -7 / 2` is never driven, because the driver has no `sw_i(-3)`.
5. No test covers the integer branch of `FoldArithmetic` (cast-of-float operands such as `(int)1.5 + 2147483647`) or unsigned-to-double conversion. Both gave correct values in the reviewer's probes.
6. Still not constants, as before this change: `&&`, `||` and `?:` (`int g = 1 && 2;`, `case 1 ? 2 : 3:`), and casts of floats in `case`, enum and array-size contexts (`char a[(int)2.5]`).

## Open after v3 step 1

From the step 1 review (2026-11-11, approved in round 2). These are design questions for Marcus. None blocked the merge.

1. DECIDED (D23 a): next to `EmitRISC` in each node's file; step 2 moves the step 1 code. **Where the lowering code lives (decide before step 2).** All `EmitIR`/`EmitValue` code is in one file, `src/ast_ir.cpp` (D22(b)), about 300 lines now and likely 1500+ by step 6. The options are to put each `EmitIR` next to its node's `EmitRISC` in the existing `src/ast_*.cpp` files, or to split by group (`ast_ir_operators.cpp`, `ast_ir_stmt.cpp`, ...). Step 7 deletes functions either way.
2. DECIDED (D23 b, 21.10): `ptradd`, `ptrtoint`, `inttoptr`, in step 4. **Pointer arithmetic in the IR (decide before step 4).** 21.2 has no pointer-to-integer conversion, and the checker rejects `Ptr` in arithmetic. The reviewer suggests a `ptradd ptr, iN` (byte offset already scaled by the frontend) plus `ptrtoint`/`inttoptr`. This needs a D21 amendment.
3. DECIDED (D23 c): partial dump, done in step 2. **Partial dump when lowering throws.** `--emit-ir` prints nothing when a node throws `not in IR yet` (D22(h)). Cheap option: print the finished functions and the partly built one, then rethrow.
4. **Trailing `ret`.** Every function ends with an implicit `ret`, even after an explicit one. One option is to skip it when the last instruction is already a `ret`.
5. REQUIRED BEFORE AArch64 (D23 d). **The `.L` label prefix comes from the frontend** (`Context::NewLabel`, `src/context.cpp:142`). It is assembler syntax, so the backend should add it.
6. **Old-path coupling to remove before step 7.** `FunctionDefinition::EmitIR` calls `ctx.BeginFunction` (old frame counters), and `src/backend_riscv.cpp` includes `emit.hpp` for `EmitMem`/`EmitAddrOffset`.
7. **Unary minus on a constant is not folded** (`-1` becomes `neg` then `trunc`). Correct, but it makes the dump noisy.
8. **Narrow returns are rejected under `--ir`** (`src/ast_ir.cpp:283-290`), unsigned ones included. The step 3 classifier must remove this guard; `tests/custom/ir_narrow_return` is the test for it.
9. Commit `147dee6`'s message reads "(default 283/283 + 1 new custom test, --ir 0→18)" instead of "(default 284/284, --ir 0→18)". It was left as is, to avoid rewriting history.

## Required before an AArch64 backend

1. Move `.L` label naming to the backend. Today `Context::NewLabel` (`src/context.cpp:142`) adds the `.L` prefix in the frontend, which is RISC-V assembler syntax. IR labels should be plain names, and each backend adds its own local-label prefix (Marcus, 2026-11-11, D23 d).

## Nice to have from the v3 step 2 review

From the step 2 review (2026-11-11, approved in round 1). None of these blocked the merge.

1. **Checker does not enforce the one-site rule** (`src/ir_check.cpp:85-98`, `Define`). Any instruction may redefine a temp, where 21.2 allows only the `?:` temp two sites. Rule to add: a temp defined more than once must be defined only by `copy`/`const`. It holds today but is unchecked. Recommended before step 3.
2. `&&`/`||` `EmitValue` (`src/ast_operators.cpp`) hard-code `IrType::I32` instead of `IRGen::ToIr(*GetType(ctx))`, the D22(b) contract. Harmless while `int` is 32 bits.
3. `Context::AddCase` (`src/context.cpp:270-280`) wraps case values to `Type::Int()`, not to the promoted controlling type. Wrong once `long` is parsed and is `I64` on a 64-bit target.
4. **Step 7 must keep or move, not delete as dead:** the IR path now uses `Context::PushLoop`/`PopLoop`/`BreakLabel`/`ContinueLabel`, `PushSwitch`/`PopSwitch`/`AddCase`/`AddDefault`/`SwitchInfo`, and `NewLabel`. The backend's `beqz t0, 1f` relies on GNU numeric local labels (relevant to the AArch64 label item).
5. Commit `3252e7f`'s message says "large immediates in the RISC-V backend", but the backend already handled them; the commit adds a comment and a test. Left as is, to avoid rewriting history.
6. Tidiness: a whitespace-only line before `Condition` in `src/ast_lower.cpp`; `src/ast_lower.cpp` repeats the comments in `include/ast_lower.hpp`; an odd line break in the comment in `CompileViaIR` (`src/main.cpp:70-75`).
