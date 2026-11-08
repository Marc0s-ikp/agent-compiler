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

Status: all of the above was approved by Marcus on 2026-11-08.

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
