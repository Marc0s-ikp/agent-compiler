# Design decisions

| Date | Decision | Reason | Alternatives considered |
|------|----------|--------|-------------------------|
| 2026-11-08 | ACCEPTED D1: every expression evaluates into one accumulator (`a0` or `fa0`), with intermediates saved in frame temp slots | No allocator and no live-register state to get wrong around calls. Speed is not tested | Register pool with spilling (more code, faster output, not needed for these tests); destination register passed down the tree |
| 2026-11-08 | ACCEPTED D2: frame addressed from `s0`, `sp` fixed after the prologue, function body buffered so the prologue is written last | Frame size is only known after the body. A fixed `sp` keeps 16-byte alignment and makes outgoing stack args simple | Pre-pass to compute frame size (a second tree walk to keep in sync); push/pop on `sp` (alignment bugs around calls) |
| 2026-11-08 | ACCEPTED D3: `Type` is an immutable shared value object; `Expr::GetType` is pure and emits nothing | Types first-class from day one. `sizeof expr` must not generate code | Storing a type on each node at parse time (needs scopes the parser doesn't have) |
| 2026-11-08 | ACCEPTED D4: separate `EmitRISC` (value) and `EmitAddress` (lvalue) on `Expr` | One rule for assignment, `&`, `++`, members and indexing | A boolean "want address" flag through `EmitRISC` |
| 2026-11-08 | ACCEPTED D5: one ABI classifier shared by caller and callee | Both sides of a call cannot disagree | Separate logic in `FunctionCall` and `FunctionDefinition` |
| 2026-11-08 | ACCEPTED D6: one conversion routine (`EmitConvert`) for all implicit and explicit conversions | Assignment, return, args, binary ops and casts behave the same | Conversions written inside each node |
| 2026-11-08 | ACCEPTED D7: lexer asks a parse-time scoped `TypedefTable` whether an identifier is a type name | C's grammar needs this to be unambiguous, and it must exist before `Context` does | Using `Context` during parsing (couples parsing and code generation) |
| 2026-11-08 | ACCEPTED D8: plain `char` is unsigned, matching this GCC target | `__CHAR_UNSIGNED__` is defined for rv32imfd/ilp32d, and test drivers are built by GCC | Signed `char` (the x86 habit, wrong here) |
| 2026-11-08 | ACCEPTED D9: function declarations without a body record a `Function` symbol and emit no storage or label. Calls to undeclared functions default to an `int` return | Added in Marcus's review. A prototype is type information only, and C90 allows implicit declarations | |
| 2026-11-08 | ACCEPTED D10: expressions of array type are never loaded. They evaluate to their address, through `Decay` | Added in Marcus's review. An array has no scalar value to load | |
| 2026-11-08 | ACCEPTED D11: global initialisers are compile-time constants emitted as `.data`/`.rodata`, covering integer, float/double and address-of-global | Added in Marcus's review. Globals are initialised before any code runs | Extending `EvalConst` or adding a separate constant evaluator; the choice is left open until feature 5 |
| 2026-11-08 | ACCEPTED D12: `FrameSize()` rounds to 16 including the outgoing area. Temp and local slots respect type alignment (8 for `double`) | Added in Marcus's review. Keeps `sp` ABI-aligned and every slot correctly aligned | |
| 2026-11-08 | ACCEPTED D13: when each feature group is reached, `tester` writes `tests/custom/` cases for anything the reference tests don't cover | Added in Marcus's review. The reference tests leave parts of the feature order untested | |
| 2026-11-08 | ACCEPTED D14: bison semantic values are raw pointers in %union with %destructor; ownership moves to std::unique_ptr when a node adopts them | bison's C++ variants need the lalr1.cc skeleton and a different lexer interface; design section 1 says unique_ptr throughout, so this states the exception for the parser stack | lalr1.cc skeleton with variants |
| 2026-11-08 | ACCEPTED D15 (Marcus, 2026-11-09): address constants for global initialisers (resolves the open choice in D11 and section 6a). `EvalConst` stays integer-only. Two small virtuals on `Expr` return a `ConstValue` (label plus byte offset; empty label means a plain integer): `EvalConstValue` (the value, defaulting to `EvalConst`) and `EvalConstAddress` (what `&` yields). They are overridden by `Identifier`, `AddressOf`, `Index` and `BinaryArith`. `BuildGlobalInit` (`src/global_init.cpp`) walks the initialiser and writes `.word label+off`, `.zero` etc. Pointer offsets scale by `PointeeSize`. Storage for all globals is written once at the end of the file by `Context::EmitGlobals`, so tentative definitions and `extern` redeclarations merge into one object. Amendment (Marcus, 2026-11-09): `StringLiteral` also overrides `EvalConstValue` / `EvalConstAddress`, so `char *s = "hi";` emits a `.word` to the string's label, and `char *p = "hello" + 1;` works through the existing `BinaryArith` override. `char s[] = "hi";` emits inline bytes plus NUL with the array size inferred (3), does not go through the address path and leaves no entry in the string pool | The evaluator needs node internals (`Index` is a sum, `AddressOf` an operand), so a virtual per node fits the existing `EvalConst` pattern; it reuses `EvalConst` and `PointerOp` rather than duplicating them. Float/double (feature 7) will add a case in `InitWriter::Scalar` and a floating value alongside `ConstValue` without touching the walk | Making `EvalConst` return a variant (touches every override and the array-size caller); a free-standing evaluator that downcasts nodes (needs accessors on every class) |
| 2026-11-08 | ACCEPTED D16 (Marcus, 2026-11-09): how D7 is built. `Typedefs()` (`typedef_table.hpp`) maps name to "is a type" per scope. `{` and `}` push and pop it through mid-rule actions in `compound_stmt`. A declarator is registered as soon as it is complete (`declarator_head`), as a type inside `typedef`, otherwise as an ordinary name. Declarator names accept `T_TYPE_NAME` so `int T;` can hide a typedef. Parameter names wait in a pending list and enter the function body scope at its `{`; `EndDeclaration()` drops them for prototypes | Bison takes default reductions without reading a lookahead, so the scope actions and the `;` actions run before the next identifier is lexed | Registering names in the AST pass (the lexer would classify too early); one flat table (wrong after a block closes). Known limit: parameters of a function-pointer parameter, as in `f(int (*cb)(int x))`, also land in the pending list |
| 2026-11-09 | D17 (implementer, accepted by Marcus 2026-11-09): feature 6 plumbing. (a) `EvalConstValue` and `EvalConstAddress` take `Context&` (not const), and so does `BuildGlobalInit`, because a string literal's address constant interns the string in the pool (`Context::AddString`, which returns the same label for the same bytes). `GetType` and `EvalConst` stay const and pure (D3), so `sizeof` can never touch the pool. (b) `TypeSpecifier::Resolve(Context&)` declares enum bodies; `TypeSpecifier::Lookup(const Context&)` only reads, and rejects an enum body. `Declarator::Apply` and `ResolveParams` take `const Context&` and use `Lookup`, so `sizeof(type)` works from `GetType`/`EvalConst`. (c) One `FlattenInit(type, init)` (`src/init_layout.cpp`) lays an initialiser out as a list of (offset, scalar expression or string bytes) items, applying nesting, partial lists, brace elision, `{x}` for scalars and string-for-char-array once. The global writer (`src/global_init.cpp`) turns the items into `.word`/`.ascii`/`.zero`; the local emitter (`src/local_init.cpp`) zeroes the array and stores each item. `Expr::StringBytes()` lets an initialiser tell `char s[] = "ab"` (bytes inline) from `char *s = "ab"` (address). (d) Plain `char` is unsigned (D8), so `char`, `unsigned char` are the same type and `signed char` is the signed one; char constants have type int | (a) keeps the existing virtual-per-node pattern of D15 and needs no `const_cast`; the cost is that the two address-constant virtuals are no longer const-context. (b) and (c) keep one rule in one place instead of one copy per caller | (a) a `mutable` pool or `const_cast` (hides a side effect behind a const reference); registering strings in a pre-pass over the tree (a second walk to keep in sync); returning the string bytes in `ConstValue` and interning later in the writer (the same pool access, one step further away). (c) a second, separate walker for locals (the elision rules would exist twice) |
| 2026-11-09 | D18 (implementer, accepted by Marcus 2026-11-09): feature 7 plumbing, filling in D1, D5, D6, D11 and D15 without changing them. (a) `ConstValue` gains `is_real` and a `double real`; the flag is set exactly when the expression's type is float or double, and a float constant holds a value float can represent. `ConvertConst(value, to)` (`src/const_value.cpp`) is the compile-time twin of `EmitConvert` (truncate toward zero, wrap to width, round to float), and `InitWriter::Scalar` calls it for every scalar, so `float f = 1;`, `int i = 2.9;` and a double constant in a float all convert once, in one place. `FloatLiteral`, `UnaryOp`, `BinaryArith`, `Cast` and now `BinaryCompare` override `EvalConstValue`; `EvalConst` stays integer-only, and `Cast::EvalConst` folds integer-to-integer casts. (b) Accumulator rule: `AccReg(type)` is `fa0` for float/double, else `a0`, and `ScratchReg` is `ft0`/`t0`; `EmitLoad`/`EmitStore` take the register from the caller and pick `flw/fld/fsw/fsd` from the type. Every node asks `AccReg` rather than printing `a0`. (c) `EmitCondition` is the one place that turns a value into a truth test: a float is compared with zero by `feq` then `xori`, so -0.0 is false and NaN is true; `!` goes through it too. Comparisons use `flt/fle/feq` (`>` and `>=` swap operands, `!=` inverts `feq`). (d) `ClassifyArgs` kinds: `IntReg`, `FloatReg`, `IntPair`, `Split`, `Stack`. A double with no fa register takes two consecutive a registers starting wherever the next one is (no even-register alignment: that rule is for varargs, and GCC does the same), or `Split` (a7 low word, one stack word) or an 8-aligned stack slot. A float with no fa register goes to an a register or a 4-byte stack slot as its 32 bits. The callee copies a `Split` double to its 8-aligned home with integer stores, so it never does a misaligned `fld`. (e) Float literals are pooled with `Context::AddFloat(bits, size)` (bits from `strtof` for an `f` suffix, `strtod` otherwise, so a float is rounded once), emitted in `.rodata` after the strings. (f) `Cast` (`src/ast_cast.cpp`) uses `EmitConvert`. The grammar gets a `cast_expr` level between binary and unary operators, as in the standard C grammar, so `sizeof (type)` and `(type) expr` do not conflict. (g) `UsualArithmetic` returns the wider floating type if either operand is floating; `float` is not promoted to `double` except as an argument with no declared parameter. `IsScalar` now includes floating types | Keeps D6 (one conversion routine, run-time and compile-time) and D1 (no live register state): a float temp is stored from `fa0` exactly as an int temp is stored from `a0`. The cast level and the `ClassifyArgs` kinds are the only structural additions, and both are what D5 and the design's section 6 already described | (a) a variant return from `EvalConst`; folding in a separate pass. (d) even-register alignment for pairs, which GCC does not do for named arguments; reassembling a split double with `fld` (misaligned under spike pk)  |
| 2026-11-10 | D19 (implementer, awaiting Marcus): feature 8, structs and unions, filling in the design's `Record`, `Member`, tag namespace and struct-as-address convention. (a) **Record identity.** `Type::Record(is_union, tag)` makes an incomplete record; `Type::DefineRecord` lays out the members once (struct: next offset the member's alignment allows, size rounded up to the largest alignment; union: every offset 0). The members live in a shared `RecordData`, so a pointer made before completion sees the members, and `Equals` on two records is identity of that `RecordData`, never structural, so `struct node { struct node *next; }` cannot recurse. A record reaches itself through a member and shared pointers never free a cycle, so `Context::NewRecord` owns every record and its destructor calls `Type::ReleaseRecord` (the Makefile links LeakSanitizer). Tags use `DeclareTag`/`FindTag`/`FindTagHere`: a use without a body searches outwards and declares an unknown tag incomplete in the current scope; a body, or a bare `struct T;`, searches only the current scope, so an inner definition hides an outer one. A struct body is accepted only where `Resolve` can declare (declarations, member declarations); `Lookup` (type names, parameter lists) accepts `struct T` for a known tag and rejects a body, as for enums (D17). Member declarators are parsed by `declarator` without `declarator_head` or `EndDeclaration`, and `.` and `->` accept `T_TYPE_NAME` as the member name. (b) **Values.** An expression of struct or union type evaluates to the address of an object that holds it (`EmitLoad` does nothing for a record, as for an array), so `Member` needs only the base's `EmitRISC` plus an offset, and `f().x`, `(c ? a : b).x`, `(a = b).x` need no special case. A consumer that keeps the value copies it at once: `Assign` (the target's address is the result), an argument temp, `return`, an initialiser. `EmitCopyMem` is the one copy routine (word or byte units from the struct's alignment, unrolled to 16 units, then a loop); the byte-exact piece loads and stores (`EmitLoadBytes`/`EmitStoreBytes`) never read or write past an object or use a misaligned word. (c) **Temp lifetime.** `PushTemp`/`PopTemp` stay strictly nested, so a struct returned by a call cannot live in a temp: `FunctionCall` allocates its result with `AllocLocal`, a slot that lasts to the end of the function, and `PopTemp` never releases below a new `floor_` that `AllocLocal` raises. Each call site has its own slot, so `g(f(1), f(2))` keeps both results, and a loop reuses its site's slot. The cost is frame space, one slot per struct-returning call site; argument copies are still ordinary temps, popped after the call. (d) **`ClassifyArgs`.** `ArgLoc` gains `Kind::Fields` (a floating-convention struct: one `FieldLoc` per flattened scalar with its own `fa`/`a` register), a `by_reference` flag (the one-word pointer to the caller's copy is in `IntReg` or `Stack`), and `ClassifyArgs(types, hidden_return)`, where a hidden result pointer takes a0. Up to 8 bytes in the integer convention reuses `IntReg`, `IntPair`, `Split`, `Stack` exactly as a double does (no even-register alignment). The floating convention (structs only, never unions; one or two scalars after flattening nested structs and arrays; at least one floating; integers up to 4 bytes, and pointers disqualify it, as in GCC) needs every register it asks for, else the whole struct falls back to the integer convention. `ArgPieces` and `ClassifyReturn` turn a location into register `Piece`s (offset, byte count, register); `src/record_abi.cpp` holds the four functions that use them, in caller/callee pairs, so neither side has its own rules. Over 8 bytes in the integer convention is by reference; returns over 8 bytes use the hidden pointer, which the callee keeps in a frame slot and returns in a0. The callee copies a by-reference parameter into its own home slot (one extra copy; the ABI lets it use the caller's directly). (e) **Initialisers.** `FlattenInit` takes the `Context`, since deciding between a struct-valued expression and brace elision needs the expression's type, and gains `Record` (members in order, a union only its first) and `RecordObject`; a struct-valued expression is one `InitItem` of record type, which the local emitter copies and the global writer rejects as not constant. Locals zero the whole struct first, as arrays, so unlisted members and padding are zero | Keeps D1 (nothing live across a sub-expression: a struct is an address and every consumer copies), D3 (types pure), D5 (one classifier for caller and callee) and D17 (one initialiser walk). Identity plus an owner for cleanup is the smallest answer to self-reference. A call-site slot is the simplest lifetime that is obviously right for any nesting | (a) tag lookup that always searches outwards (breaks `struct T { }` shadowing); breaking cycles with `weak_ptr` in members (every `Base()` would need a lock). (b) a separate `EmitStructAddress`; copying a struct into a0/a1 registers as the value. (c) a deeper temp stack with a release point per full expression (more state in every statement node); one hidden local per function reused by every call. (d) representing every struct as one `ArgLoc` per word (loses the floating convention); having the callee use the caller's by-reference copy in place (needs an indirect symbol kind). (e) a second walker for struct initialisers |

Status: all of the above was approved by Marcus (D15 to D18 on 2026-11-09, the rest on 2026-11-08).

---

# Design detail (accepted 2026-11-08)

## Scope, from the reference tests

There are 86 tests in `tests/compiler_tests/`. Surveyed by text search, they use `int`, `char`, `float`,
`double`, `unsigned`, pointers, arrays, one-level `struct`, `enum`, `typedef`, `switch` and `sizeof`.
None of them uses `union`, `static`, `extern`, `goto`, function pointers, variadics, or struct by-value
arguments.

CLAUDE.md lists `static` and unions in the feature order, so the design covers them, but they get no
test signal from this suite. `long double` and `long long` are out of scope.

## Target facts (checked against the toolchain in the Docker image)

| Type | Size | Align |
|------|------|-------|
| `char` (unsigned) | 1 | 1 |
| `short` | 2 | 2 |
| `int`, `long`, pointer, enum, `float` | 4 | 4 |
| `double` | 8 | 8 |

## 1. AST class hierarchy

All nodes live in `namespace ast`. Each construct gets one class, in `include/ast_*.hpp` with its
implementation in `src/ast_*.cpp`. `src/` must stay flat because the Makefile's wildcard doesn't recurse.

```
Node                         EmitRISC(ostream&, Context&) const; virtual ~Node
├── TranslationUnit
├── ExternalDecl
│   ├── FunctionDefinition
│   └── GlobalDeclaration
├── Stmt
│   ├── CompoundStmt, ExprStmt, DeclarationStmt, IfStmt, WhileStmt, DoWhileStmt, ForStmt
│   ├── ReturnStmt, BreakStmt, ContinueStmt
│   └── SwitchStmt, CaseStmt, DefaultStmt
└── Expr                     + GetType(const Context&) const -> TypePtr
    │                        + EmitAddress(ostream&, Context&) const   (default: "not an lvalue" error)
    │                        + EvalConst(const Context&) const -> optional<long>
    ├── Literals:    IntLiteral, FloatLiteral, CharLiteral, StringLiteral
    ├── Names:       Identifier
    ├── Operators:   BinaryArith, BinaryCompare, LogicalAnd, LogicalOr, UnaryOp, IncDec, Ternary, Comma
    ├── Assignment:  Assign, CompoundAssign
    ├── Access:      Index, Deref, AddressOf, Member (covers both `.` and `->`)
    ├── Types:       Cast, SizeofExpr, SizeofType
    └── FunctionCall

Parse-only helpers (not code-generating `Node`s):
  TypeSpecifier    int / char / struct x {...} / enum {...} / typedef name; Resolve(Context&) -> TypePtr
  Declarator       Name, Pointer, Array, Function; Apply(base TypePtr, Context&) -> (name, TypePtr)
  Initializer      single Expr, or brace list
```

Ownership is `std::unique_ptr` throughout. The Makefile links LeakSanitizer, so a leak on a successful
compile makes the compiler exit non-zero, which `run_tests.sh` records as a CRASH.

`Declarator::Apply` builds types inside-out, which is what makes `int *a[3]` and `int (*a)[3]` come out
different. Type specifiers resolve during code generation, because array bounds, enum values and struct
tags need scope information.

## 2. Types

`using TypePtr = std::shared_ptr<const Type>`, built once and never modified.

| Kind | Holds |
|------|-------|
| `Void` | |
| `Integer` | size, signedness |
| `Floating` | size (4 or 8) |
| `Pointer` | pointee |
| `Array` | element, length |
| `Record` | struct or union, tag, members (name, type, offset), size, align |
| `Function` | return type, parameter types |

Enums are `int`. Every `Type` answers `Size()`, `Align()` and the predicates `IsInteger`, `IsFloating`,
`IsPointer`, `IsScalar`. Free functions hold the C rules: `Promote`, `UsualArithmetic(a, b)`, and
`Decay` (array to pointer, function to pointer).

`EmitLoad(type)` and `EmitStore(type)` are the only places that pick `lw`/`lbu`/`lhu`/`lh`/`flw`/`fld`
and their stores. `EmitConvert(from, to)` is the only place that emits `fcvt.*` and truncation.
Signedness picks `div`/`divu`, `rem`/`remu`, `sra`/`srl`, `slt`/`sltu` in `BinaryArith` and
`BinaryCompare`. Pointer arithmetic scales by `pointee->Size()` in one helper.

Expressions of array type are never loaded. They evaluate to their address, through `Decay` (D10).

## 3. Context

```
class Context
  Scopes        PushScope / PopScope / Declare(name, Symbol) / Lookup(name)
                DeclareTag / LookupTag  (struct and enum tags, own namespace)
  Frame         BeginFunction / AllocLocal(type) -> offset
                PushTemp(type) -> offset / PopTemp()
                ReserveOutgoing(bytes) / FrameSize()
  Control       Push/PopLoop(break, continue) / Push/PopSwitch(break)
                BreakLabel() / ContinueLabel()
                CurrentSwitch().AddCase(value, label)
  Function      ReturnLabel() / ReturnType()
  Labels        NewLabel(prefix)  -> ".L<prefix><n>", unique per file
  Data pool     AddString(text) -> label / AddFloat(bits, size) -> label
                EmitDataPool(ostream&)   after all functions

struct Symbol { kind: Local | Global | Function | EnumConst; TypePtr type;
                int offset; string label; long enum_value; }
```

Inner scopes shadow outer ones by search order, innermost first. Parameters are ordinary `Local`
symbols, so `Identifier` has only three storage cases (local, global, enum constant).

Loops and switches share the break logic but only loops set a continue target, so `continue` inside a
`switch` inside a loop finds the loop.

`switch` uses the same buffering trick as functions: emit the body to a string, with each `CaseStmt`
registering its value and label as it goes, then emit the dispatch followed by the buffered body.

## 4. Stack frame

```
higher addresses
  incoming stack args      s0 + 0, s0 + 4, ...
  ------------------------ s0  (the caller's sp)
  saved ra                 s0 - 4
  saved caller s0          s0 - 8
  locals and register-arg homes
  temp slots
  outgoing stack args      sp + 0, sp + 4, ...
  ------------------------ sp  (16-byte aligned, fixed after the prologue)
lower addresses
```

- `FunctionDefinition` emits the body into an `ostringstream`, then the prologue with the now-known
  frame size, the body, the return label and the epilogue.
- `FrameSize()` rounds to 16 including the outgoing area. Temp and local slots respect type alignment,
  which is 8 for `double` (D12).
- On entry, each register argument is copied to a local home slot. After that, parameters are ordinary
  locals and argument registers are free. Stack-passed parameters stay where the caller put them.
- All frame access goes through one helper. If the offset does not fit a 12-bit immediate (beyond
  -2048..2047, which a large local array reaches), it builds the address in `t6` first. The prologue
  and epilogue do the same.

## 5. Register strategy

| Register | Use |
|----------|-----|
| `a0` | Integer, pointer result of any expression. Address result of `EmitAddress`. Address of a struct value |
| `fa0` | `float` or `double` result of any expression |
| `t0`, `t1` | Secondary operand and scratch, within one node only |
| `ft0`, `ft1` | Floating secondary operand and scratch, within one node only |
| `t6` | Reserved for large-offset addressing |
| `a0`-`a7`, `fa0`-`fa7` | Argument passing, loaded immediately before `call` |
| `s0`, `ra` | Frame pointer, return address. Always saved and restored |
| `s1`-`s11`, `fs0`-`fs11` | Never used, so never need saving. This meets the callee-saved rule by construction |

Binary operators follow one pattern:

```
emit left   -> a0
store a0 to temp slot
emit right  -> a0
mv t0, a0;  load temp slot -> a0
op a0, a0, t0
```

No register holds a value across a child's `EmitRISC`, so a nested call cannot corrupt anything. The
cost is slower generated code. The largest reference test is 19 lines, and the suite's timeout is 5
seconds per step.

`LogicalAnd`, `LogicalOr` and `Ternary` use branches and evaluate only what C requires.
`CompoundAssign` and `IncDec` save the operand's address in a temp slot, so it is evaluated once.

## 6. Calls (ilp32d)

`ClassifyArgs(paramTypes) -> vector<ArgLoc>` is used by both `FunctionCall` and `FunctionDefinition`.
Each `ArgLoc` is an integer register, a floating register, a register pair, or a stack offset.

| Argument | Placement, in order of preference |
|----------|-----------------------------------|
| Integer, pointer | `a0`-`a7`, then 4-byte stack slot |
| `float` | `fa0`-`fa7`, then `a0`-`a7`, then stack |
| `double` | `fa0`-`fa7`, then two `a` registers (may split: last register plus stack), then 8-aligned stack |

The caller evaluates every argument into a temp slot first, since an argument may itself contain a
call. It then copies them to their outgoing locations and issues `call`. Return values use `a0` or
`fa0`.

Struct arguments and returns by value are not in the reference tests. They are left out at first, and
`ArgLoc` can be extended when feature 8 is reached.

A function declaration without a body records a `Function` symbol and emits no storage or label. A call
to an undeclared function defaults to an `int` return (D9).

## 6a. Global initialisers

Global initialisers are compile-time constants emitted as `.data`/`.rodata`. They cover integer,
float/double and address-of-global values (D11). Either `EvalConst` is extended or a separate constant
evaluator is added, decided at feature 5. Note that `EvalConst` currently returns `optional<long>`, which
cannot represent floating or address values.

## 7. Parsing

- `TypedefTable` is a small scoped set updated by the parser's declaration actions and read by the
  lexer to choose `T_TYPE_NAME` over `T_IDENTIFIER`.
- `src/lexer.flex` needs `%option noyywrap nounput noinput`. Without these, flex output fails the
  Makefile's `-Werror`. A stub built with those options compiled cleanly.

## 8. Testing the design against the feature order

| Feature | Lands in |
|---------|----------|
| 1 Integer expressions, locals | `BinaryArith`, `Assign`, `Identifier`, frame, temp slots |
| 2 Control flow | Statement nodes, Context control stacks, labels |
| 3 Functions | `ClassifyArgs`, `FunctionCall`, outgoing area |
| 4 Arrays, pointers | `EmitAddress`, `Declarator`, pointer-scaling helper |
| 5 Globals, enum, typedef, switch | `GlobalDeclaration`, `EvalConst`, `TypedefTable`, switch buffering |
| 6 char, strings, sizeof | `EmitLoad`/`EmitStore` by size, data pool, pure `GetType` |
| 7 float, double | `fa0` path, `EmitConvert`, data pool, classifier floating rows |
| 8 structs, unions | `Record`, `Member`, struct-as-address convention |

When each feature group is reached, `tester` writes `tests/custom/` cases for anything the reference
tests don't cover (D13).
