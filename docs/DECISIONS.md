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
| 2026-11-10 | D19 (implementer, accepted by Marcus 2026-11-10): feature 8, structs and unions, filling in the design's `Record`, `Member`, tag namespace and struct-as-address convention. (a) **Record identity.** `Type::Record(is_union, tag)` makes an incomplete record; `Type::DefineRecord` lays out the members once (struct: next offset the member's alignment allows, size rounded up to the largest alignment; union: every offset 0). The members live in a shared `RecordData`, so a pointer made before completion sees the members, and `Equals` on two records is identity of that `RecordData`, never structural, so `struct node { struct node *next; }` cannot recurse. A record reaches itself through a member and shared pointers never free a cycle, so `Context::NewRecord` owns every record and its destructor calls `Type::ReleaseRecord` (the Makefile links LeakSanitizer). Tags use `DeclareTag`/`FindTag`/`FindTagHere`: a use without a body searches outwards and declares an unknown tag incomplete in the current scope; a body, or a bare `struct T;`, searches only the current scope, so an inner definition hides an outer one. A struct body is accepted only where `Resolve` can declare (declarations, member declarations); `Lookup` (type names, parameter lists) accepts `struct T` for a known tag and rejects a body, as for enums (D17). Member declarators are parsed by `declarator` without `declarator_head` or `EndDeclaration`, and `.` and `->` accept `T_TYPE_NAME` as the member name. (b) **Values.** An expression of struct or union type evaluates to the address of an object that holds it (`EmitLoad` does nothing for a record, as for an array), so `Member` needs only the base's `EmitRISC` plus an offset, and `f().x`, `(c ? a : b).x`, `(a = b).x` need no special case. A consumer that keeps the value copies it at once: `Assign` (the target's address is the result), an argument temp, `return`, an initialiser. `EmitCopyMem` is the one copy routine (word or byte units from the struct's alignment, unrolled to 16 units, then a loop); the byte-exact piece loads and stores (`EmitLoadBytes`/`EmitStoreBytes`) never read or write past an object or use a misaligned word. (c) **Temp lifetime.** `PushTemp`/`PopTemp` stay strictly nested, so a struct returned by a call cannot live in a temp: `FunctionCall` allocates its result with `AllocLocal`, a slot that lasts to the end of the function, and `PopTemp` never releases below a new `floor_` that `AllocLocal` raises. Each call site has its own slot, so `g(f(1), f(2))` keeps both results, and a loop reuses its site's slot. The cost is frame space, one slot per struct-returning call site; argument copies are still ordinary temps, popped after the call. (d) **`ClassifyArgs`.** `ArgLoc` gains `Kind::Fields` (a floating-convention struct: one `FieldLoc` per flattened scalar with its own `fa`/`a` register), a `by_reference` flag (the one-word pointer to the caller's copy is in `IntReg` or `Stack`), and `ClassifyArgs(types, hidden_return)`, where a hidden result pointer takes a0. Up to 8 bytes in the integer convention reuses `IntReg`, `IntPair`, `Split`, `Stack` exactly as a double does (no even-register alignment). The floating convention (structs only, never unions; one or two scalars after flattening nested structs and arrays; at least one floating; integers up to 4 bytes, and pointers disqualify it, as in GCC) needs every register it asks for, else the whole struct falls back to the integer convention. `ArgPieces` and `ClassifyReturn` turn a location into register `Piece`s (offset, byte count, register); `src/record_abi.cpp` holds the four functions that use them, in caller/callee pairs, so neither side has its own rules. Over 8 bytes in the integer convention is by reference; returns over 8 bytes use the hidden pointer, which the callee keeps in a frame slot and returns in a0. The callee copies a by-reference parameter into its own home slot (one extra copy; the ABI lets it use the caller's directly). (e) **Initialisers.** `FlattenInit` takes the `Context`, since deciding between a struct-valued expression and brace elision needs the expression's type, and gains `Record` (members in order, a union only its first) and `RecordObject`; a struct-valued expression is one `InitItem` of record type, which the local emitter copies and the global writer rejects as not constant. Locals zero the whole struct first, as arrays, so unlisted members and padding are zero | Keeps D1 (nothing live across a sub-expression: a struct is an address and every consumer copies), D3 (types pure), D5 (one classifier for caller and callee) and D17 (one initialiser walk). Identity plus an owner for cleanup is the smallest answer to self-reference. A call-site slot is the simplest lifetime that is obviously right for any nesting | (a) tag lookup that always searches outwards (breaks `struct T { }` shadowing); breaking cycles with `weak_ptr` in members (every `Base()` would need a lock). (b) a separate `EmitStructAddress`; copying a struct into a0/a1 registers as the value. (c) a deeper temp stack with a release point per full expression (more state in every statement node); one hidden local per function reused by every call. (d) representing every struct as one `ArgLoc` per word (loses the floating convention); having the callee use the caller's by-reference copy in place (needs an indirect symbol kind). (e) a second walker for struct initialisers |
| 2026-11-10 | D20 (implementer, accepted by Marcus 2026-11-10): integer `EvalConst` returns the value in the canonical form for the expression's own `GetType` (`WrapToType`: signed sign-extended, unsigned zero-extended, in a `long`). `BinaryArith::EvalConst`, `FoldArithmetic` and `UnaryOp` fold through one helper (`FoldIntBinary` / `FoldUnary`): convert operands to the result type (a shift's rhs to its own promoted type), fold on `long`, `WrapToType` the result. With canonical operands `/`, `%`, `>>` and comparisons on `long` equal the signed or unsigned run-time result without separate branches; `Mul` multiplies as `uint64_t` so zero-extended operands cannot overflow. `BinaryCompare` gains an integer `EvalConst` (same `CompareType` and `Compare` as `EvalConstValue`), so comparisons work in case labels, enum values and array sizes. Enumerator values are wrapped to int, so `A = 0xFFFFFFFF` is -1. `address + n` offsets wrap in 32 bits. Case labels need no change: `Context::AddCase` already wraps to 32 bits, which is the conversion to the promoted switch type. Division by zero and shift counts outside 0..31 stay non-constant. `&&`, `||`, `?:` and floating folding are still not folded | Compile-time values then match run-time values in one rule, instead of raw 64-bit arithmetic that differed for 32-bit wrap and unsigned types | Separate signed and unsigned fold branches; folding in `long long`/`__int128` without canonicalising |
| 2026-11-11 | ACCEPTED D21 (Marcus, 2026-11-11) (v3): a linear, typed IR between the AST and code generation, so a second ISA needs only a new backend. AST nodes lower to IR (`EmitIR`) instead of printing RISC-V. Every IR temporary owns a stack slot, and the backend handles each IR instruction on its own: load operands into scratch registers, compute, store the result. Type sizes and alignment come from a `TargetInfo`. The frontend decides which arguments are passed and with which C types, and a per-target classifier decides where each one goes (the RISC-V one is D5/D18/D19). Frame layout, prologue and epilogue move to the backend. Migration happens in steps behind `--ir`, then the old path is deleted. Globals, strings and float constants become IR data entries that the backend writes to `.data`/`.rodata`. `--emit-ir` and an IR checker (run on every `--ir` compile) come in step 1. Additions from the review: `Target()` aborts if it is used before being set, and temp slots are never reused (reuse is a possible later optimisation). Full text in "D21 detail" below | D1 hard-wires RISC-V into every node, so a second ISA would mean a second `EmitX` on every node. One slot per temp keeps D1's rule (no register holds a value between two units of work), and IR instructions take the place of AST nodes as those units, so no register allocator is needed | Retargeting the AST directly (`EmitRISC` and `EmitX` per node, with every C rule written twice); a tree IR (closer to the AST, but the backend still has to walk and sequence it); SSA with an allocator (far more code than these tests need, against D1) |
| 2026-11-11 | ACCEPTED D22 (implementer, accepted by Marcus 2026-11-11 with notes: (b) and (h) are superseded by D23 (a) and (c); (c) is extended as noted in it), implementation choices in v3 step 1: (a) `TargetInfo` lives in `include/target.hpp`/`src/target.cpp`; `Type::Int/UInt/Float/Double`, pointer size, `double` alignment, `Promote`, `UsualArithmetic`, the `char`/`short` specifiers (and plain `char` signedness), `WrapToType` and the pointer-offset wrap in constant folding now read `Target()`. `long` is still not a parsed type, so `long_size` is unused. (b) `Node::EmitIR(IRGen&)` is a virtual with a default that throws `not in IR yet: <class>`; expressions add `Expr::EmitValue(IRGen&)` (returns an operand whose IR type is `ToIr(ValueType)`) and `Expr::EmitIRAddress(IRGen&)` (a `Ptr`). All lowering code is in one file, `src/ast_ir.cpp`, so step 7 can delete `EmitRISC` and this file stays. (c) Constants are immediate operands (`ret 5`, `add i32 %0, 3`); `ConvertInt` folds a constant conversion at compile time and emits `sext`/`zext`/`trunc` otherwise. Any integer value is legal as an IR immediate; each backend materialises it into a scratch register when it does not fit the instruction's encoding (Marcus, 2026-11-11), tested by a custom `--ir` test with large constants (`x + 100000`, `x & 0x12345`). `const` and `copy` exist in the IR and backend but the frontend does not use them yet (`?:` will). (d) Narrow values: a `char`/`short` value is an `I8`/`I16` temp with its own 1/2-byte slot; the backend loads narrow temps with `lbu`/`lhu` and the explicit `sext`/`zext` does the extension, so no instruction depends on a hidden sign. (e) Locals are frame objects named `<name>.<n>` (unique per function); `Symbol` got an `object` field for them (`Symbol::IrLocal`). Every local access is `addr $obj` then `load`/`store`. (f) Every function ends with an implicit `ret` (0 for a value-returning one), and the backend returns through the label `.Lret_<function>` and a shared epilogue. (g) Checker scope: arithmetic needs integer operands (or float for add, sub, mul, div); `Ptr` is allowed in compares and as an address only. Pointer arithmetic in IR is left to step 4. (h) With `--emit-ir`, a lowering error (`not in IR yet`) prints no dump, because there is no module; only checker errors come after a dump. | Keeps the tree-to-IR code in one place and the old nodes untouched; operands that are immediates avoid a `const` instruction per literal; explicit extension keeps the IR free of hidden signedness | A separate `EmitIR` file per node header (more churn now, same deletion later); `Const` instruction for every literal (twice the temps); sign-extending narrow loads in the backend (signedness would leak into `load`) |

| 2026-11-11 | ACCEPTED D23 (Marcus, 2026-11-11), decisions on "Open after v3 step 1" in BLOCKERS: (a) Lowering code (`EmitIR`, `EmitValue`, `EmitIRAddress`) goes next to `EmitRISC` in each node's `src/ast_*.cpp` file. The step 1 code moves out of `src/ast_ir.cpp` as part of step 2. Supersedes D22(b)'s single file. (b) D21 amendment (21.10): `ptradd Ptr, offset`, where the offset is a pointer-width integer byte count. The frontend scales by the element size and explicitly widens the index to pointer width (`sext` or `zext` by the index's C type). New conversions `ptrtoint` and `inttoptr`. Pointer difference is `ptrtoint` both, `sub`, then divide by the element size. Implemented in step 4. (c) When lowering throws, `--emit-ir` writes a partial dump (the finished functions and the partly built one), then the error. Done in step 2. Supersedes D22(h). (d) Moving the `.L` label prefix from the frontend (`Context::NewLabel`) to the backend is required before an AArch64 backend (BLOCKERS). | Per-node files keep each construct's old and new lowering side by side, and step 7 deletes functions, not files. An explicit `ptradd` keeps `Ptr` distinct from integers, so a 64-bit target changes nothing in the frontend | One `ast_ir.cpp`, or files per group (`ast_ir_operators.cpp`, ...); integer `add` on pointers (loses the `Ptr` type) |
| 2026-11-11 | D24 (implementer, pending Marcus), implementation choices in v3 step 2: (a) `ast_lower.hpp`/`ast_lower.cpp` hold only the lowering helpers that several nodes share (`RequireIrInteger`, `Arith`, `ValueAs`, `Condition`); every `EmitIR`/`EmitValue` is in its node's file (D23 a), and `src/ast_ir.cpp` is deleted. (b) Loops and switches reuse `Context::PushLoop`/`PushSwitch`/`BreakLabel`/`ContinueLabel`/`AddCase`/`AddDefault`, so label and duplicate-case handling is the same on both paths. (c) `switch` computes the controlling value once (converted to the promoted type), jumps to a dispatch label, lowers the body, jumps to the end, then writes the dispatch chain (`eq` + `branch` per case, then a jump to `default` or the end) after the body; this is the IR equivalent of the old path's buffering and needs no pre-scan of the body. (d) `&&`, `||` and `?:` allocate their result with `IRGen::NewTemp` (now public) and write it at two sites with `IRGen::Move` (`const` for a constant source, `copy` otherwise), as 21.2 allows. (e) `branch` is written by the backend as `beqz t0, 1f; j if_nonzero; 1: j if_zero`, so a branch reaches the whole function (a bare `bnez` reaches only +-4 KiB). (f) `AppendPartialFunction` supports the D23 c partial dump. (g) Not done: skipping the trailing implicit `ret` (it is dead only when no label follows the last `ret`). | Keeps the two paths' control-flow bookkeeping identical; no extra IR instructions beyond the existing ones | A pre-scan of the switch body for case labels; a `select` instruction for `?:`; storing the `?:` result through a frame object |
Status: all of the above was approved by Marcus (D15 to D18 on 2026-11-09, D19 and D20 on 2026-11-10, D21 on 2026-11-11, the rest on 2026-11-08). D21 step 1 is merged (2026-11-11); steps 2 to 7 remain. D22 and D23 were accepted by Marcus on 2026-11-11.

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

---

# D21 detail (ACCEPTED, Marcus, 2026-11-11)

Goal of v3: the compiler is retargetable. Adding a second ISA means writing a new backend and a new
target description. The lexer, parser, AST, types, scopes and C semantics stay untouched.

## 21.1 Pipeline

```
source -> lexer/parser -> AST -> EmitIR (frontend, target-independent) -> ir::Module
       -> Backend::Emit (per target: frame layout, ABI placement, instruction choice) -> assembly
```

- The frontend knows C: scopes, types, conversions, promotions, pointer scaling, constant folding,
  initialiser layout (D17's `FlattenInit`) and the order of evaluation. It never names a register,
  an instruction, a frame offset or an assembler directive.
- The backend knows the machine. It never sees a C type except through the ABI classifier (21.5).
- `Context` splits along that same line. Scopes, tags, control-flow targets, labels and the string
  and float pools stay in the frontend. `AllocLocal`, `PushTemp`/`PopTemp`, `ReserveOutgoing`,
  `FrameSize`, `SetReturnPointerSlot` and every s0 offset move to the backend (21.6).

## 21.2 The IR

A module holds functions and data objects. A function holds its parameters, its frame objects, its
temporaries and a flat list of instructions with labels.

**IR types** are machine-level: `I8`, `I16`, `I32`, `I64`, `F32`, `F64`, `Ptr`. Signedness is a
property of the operation, not of the type (`div`/`divu`, `shr`/`sar`, `lt`/`ltu`, `sext`/`zext`).
`Ptr` is kept separate from the integer types so that a 64-bit target can make it 8 bytes without
changing the frontend. C `long` maps to `I32` or `I64` according to the `TargetInfo` (21.3). Structs
and unions are not IR types. Following D19(b), the value of a struct expression is the `Ptr` of an
object that holds it.

**Operands** are a temporary (`%n`, typed and defined by exactly one instruction *site*, which a
loop may run many times), an integer or floating constant, a frame object (`$name`, which has a size
and an alignment but no offset yet) or a symbol (a global, function, string or float-pool label).

**Instructions** (about 20, each in one small class or one `switch` case in the backend):

| Group | Instructions |
|-------|--------------|
| Values | `copy`, `const`, `addr` (address of a frame object or symbol, plus a byte offset) |
| Arithmetic | `add sub mul div divu rem remu and or xor shl shr sar neg not` |
| Compare | `eq ne lt ltu le leu` (integer), `feq flt fle` (floating), each giving `I32` 0 or 1 |
| Conversion | `sext`, `zext`, `trunc`, `itof`/`utof`, `ftoi`/`ftou`, `fext`/`ftrunc` |
| Memory | `load T, addr`, `store T, value, addr`, `memcpy dst, src, bytes, align`, `memzero dst, bytes, align` |
| Control | `label`, `jump`, `branch value, if_nonzero, if_zero`, `ret [value]` |
| Calls | `call` (21.5) |

`switch` is lowered by the frontend into a chain of `eq` and `branch`, which is the same dispatch
that is generated today. `&&`, `||` and `?:` turn into branches, and `?:` writes its result to one
temp from both arms. That temp has two definition sites, which is the only exception to the
one-site rule, and the backend needs nothing special for it, because a temp is a slot.

**`--emit-ir`** writes the module as text instead of assembly: each data entry (21.2a), then each
function one instruction per line. It is the debugging and review tool for every step of the
migration.

**IR checker.** `CheckIR(const ir::Module&)` runs on every module before the backend sees it,
every time the compiler runs with `--ir` (and with `--emit-ir`), in every build. It is not behind
`#ifdef DEBUG`. So every `COMPILER_FLAGS=--ir` suite run checks the IR of all 283 tests. A failure
stops the compile with an error that names the function and instruction. With `--emit-ir` the order
is reversed (Marcus, 2026-11-11, step 1): the dump is written first, then the checker runs and its
errors are printed after the dump, and the compile still fails. A broken module can then still be
inspected. With `--ir` alone the checker runs before the backend, as above. It checks:
- every temporary is defined before it is used, in the order of the instruction list (the two
  definition sites of a `?:` temp both come before its use)
- operand types are consistent: the operands of an arithmetic or compare instruction have the same IR
  type, a conversion's source and result types match its kind (`sext` widens an integer, `fext`
  widens a float, and so on), an address operand is `Ptr`, a branch condition is an integer, all the
  definition sites of a temp give it the same type, and `ret` matches the function's return type
- every label that is jumped to is defined exactly once, and every symbol refers to a data entry or
  a function in the module, or to an external function

## 21.2a Module-level data

Everything that today is written as assembler data becomes an IR **data entry** in the module. The
backend writes the entries after the functions, into `.data` or `.rodata`, with its own directives.
The frontend never prints `.word`, `.zero`, `.ascii`, `.globl` or a section name.

```
struct DataEntry {
    std::string label;
    int size, align;
    bool read_only;       // .rodata vs .data
    bool internal;        // 'static' (no .globl) or a compiler-made label (string, float, static local)
    std::vector<DataItem> items;   // empty and not read-only means zero-filled (.bss-like)
};
DataItem: Bytes(string) | Int(size, value) | Float(size, bits) | Address(label, offset) | Zero(n)
```

- **Global initialisers (D11, D15, D17, D18(a)).** `BuildGlobalInit` and `InitWriter` keep the walk
  over `FlattenInit` and the `EvalConstValue`/`EvalConstAddress` virtuals. What changes is the output:
  instead of directive text, each scalar `ConstValue` becomes one item. A plain integer becomes
  `Int(size, value)` after `ConvertConst`. A real value becomes `Float(size, bits)`. A label plus an
  offset becomes `Address(label, offset)`, whose size is `Target().pointer_size`. String bytes for
  `char s[] = "hi"` become `Bytes`, and gaps and padding become `Zero`. The rules for merging
  tentative definitions and `extern` declarations (D15) stay in the frontend's global table, and that
  table produces exactly one entry per object.
- **`static` locals** are entries with `internal` set and the label from `AddStaticLocal`.
- **String literals.** `Context::AddString` keeps the pool and its deduplication. Each pooled string
  becomes a read-only, internal entry with one `Bytes` item that includes the NUL. Code refers to it
  with `addr @label`.
- **Float constants (D18(e)).** `AddFloat(bits, size)` keeps its deduplication. Each constant becomes a
  read-only, internal entry with one `Float(size, bits)` item. Code loads it with `addr` + `load F32/F64`.
  The IR also allows a floating `const` operand. The frontend uses the pool, so the backend can
  print a float constant without knowing how its ISA materialises an immediate. Another backend could
  later handle a floating `const` differently.

The backend chooses each entry's section from `read_only`, and its alignment directive from `align`,
in one function (`WriteData`). That function is the only place on the new path that writes data
directives.

## 21.3 Target description

```
struct TargetInfo {
    int char_size, short_size, int_size, long_size, pointer_size;   // align = size unless overridden
    int float_size, double_size, double_align;
    bool char_is_signed;        // D8: false on riscv32
    int stack_align;            // 16 on ilp32d
};
void SetTarget(const TargetInfo&);  // called once in main, before parsing
const TargetInfo& Target();         // aborts if SetTarget has not been called
```

- `Target()` before `SetTarget` is a compiler bug. It prints an error naming the problem and calls
  `std::abort()`. It never falls back to a default target, so a missed initialisation cannot quietly
  produce RISC-V sizes on another target. A second `SetTarget` also aborts.
- `main` selects the target with `--target` (only `riscv32` exists, and it is the default when the
  flag is absent). `run_tests.sh` already forwards a `TARGET` variable for later use.

- `Type::Int()`, `Type::Pointer()`, `Type::Align()` for scalars, `Decay`, `UsualArithmetic`,
  `WrapToType` (D20) and the "address + n wraps in 32 bits" rule in D20 all read `Target()` instead
  of the literal constants in `type.hpp`/`type.cpp`. The table in "Target facts" above becomes the
  RISC-V `TargetInfo` value.
- `Target()` is a set-once global, like `Typedefs()` (D16). The type factories are static, and
  `GetType`/`EvalConst` take `const Context&`. Threading a target through every one of those
  signatures would touch all nodes and gain nothing, because a compile only ever has one target.
- This part also changes the old path. It is done first and is checked by the 283 tests with the
  default (old) path.

## 21.4 Temporaries and D1

A temporary maps to its own frame slot, with the size and alignment of its IR type. For each
instruction the backend loads the operands into fixed scratch registers (RISC-V: `t0`, `t1`, `ft0`,
`ft1`), computes, and stores the result to the destination's slot. Nothing is held in a register
from one instruction to the next, so a call between two instructions cannot corrupt anything. This
is D1's rule with "AST node" replaced by "IR instruction". It needs no allocator and no liveness.

**One slot per temporary, with no reuse (accepted by Marcus).** That makes frames larger than today's
nested `PushTemp`/`PopTemp` frames, and it adds one more correct but slower load and store per
operation. Large offsets already go through `t6` (design section 4).

*Possible later optimisation, not part of v3:* reuse a slot once its temp is dead. That needs a
last-use scan per function and lives entirely inside the backend's frame layout, with no change to
the IR or the frontend. It should be done only if a test or a new target needs smaller frames.

## 21.5 Calls and the ABI

**Target-independent (frontend).** The frontend decides *which* arguments are passed and *with which
C types*: it evaluates each argument into a temp in source order, decays arrays and functions,
converts to the declared parameter type, and applies the default promotions where no parameter is
declared (D18(g): `float` goes to `double`). A struct argument is a `Ptr` to a copy the frontend
makes in a frame object, as D19(b) does today. The IR instruction is

```
call  callee, result?, [(value, C type), ...], return C type, result_object?
```

where `result_object` is the call-site frame object that receives a returned struct (D19(c)).

**Per-target classifier (backend).** Each target implements

```
CallLayout Classify(const std::vector<TypePtr>& params, const TypePtr& ret);
struct CallLayout { std::vector<ArgPlace> args; RetPlace ret; int stack_bytes; };
struct ArgPlace   { bool by_reference; std::vector<Piece> pieces; };
```

`Piece` is the existing one from `abi.hpp` (offset in the value, byte count, signedness, and an
integer register, a floating register or a stack offset). Register *numbers* are indices into the
target's argument registers, and only the backend turns them into names. Every RISC-V kind already
reduces to pieces: `IntReg`, `FloatReg`, `IntPair`, `Split` (a7 plus one stack word), `Stack` and
`Fields`. The current `ClassifyArgs`, `ArgPieces`, `ClassifyReturn` and `StackExtent` become the
RISC-V classifier with their rules unchanged (D5, D18(d), D19(d)), and `record_abi.cpp` becomes its
caller/callee helpers. D5 still holds, and is now enforced inside each backend: the backend lowers
the IR `call` and the function's own entry from the same `Classify` call.

The hidden struct-return pointer is decided by the classifier and stays out of the IR, because
where it goes is target-specific (a0 on RISC-V, a dedicated register on some other ABIs).

## 21.6 Frame layout, prologue and epilogue (backend)

The backend receives a complete `ir::Function`, so the frame size is known before anything is
printed. D2's buffering trick (emit the body to a string, then the prologue) is no longer needed for
functions. D2's choices of addressing from `s0`, a fixed `sp` and the outgoing area at `sp`, and D12's
rounding to 16 with aligned slots, become the RISC-V backend's layout rules:

1. saved `ra`, `s0` (and any callee-saved registers the backend uses, which today is none)
2. homes for register parameters, and the struct-return pointer slot
3. frame objects (locals, call-site struct results, argument copies), each aligned
4. one slot per temp
5. outgoing argument area = the largest `stack_bytes` over the function's calls

The prologue then copies each incoming register piece to its parameter's home, exactly as today.

## 21.7 Migration

- `main.cpp` gains `--ir`. Without it, the old `EmitRISC` path runs, unchanged. With it:
  AST → `EmitIR` → RISC-V backend. A node that has no `EmitIR` yet throws "not in IR yet", so a test
  that uses it fails under `--ir` and keeps passing without it.
- **Measuring the IR path:** `COMPILER_FLAGS=--ir ./scripts/run_tests.sh`. Marcus added `COMPILER_FLAGS`
  and `TARGET` to the runner, and both are forwarded into Docker (commit `dbf2bc7`). Agents still may
  not edit the runner (CLAUDE.md rule 8).
- **Merge rule, every step:** the default path stays at 283/283, and the `--ir` pass count never
  goes down. Both numbers go in the commit message and in `docs/PROGRESS.md`.
- **Steps** (the "v3 migration" order in CLAUDE.md):
  1. Expressions and locals: `TargetInfo` with the aborting `Target()` (on the old path first), IR
     core, `--emit-ir`, the IR checker (on every `--ir` run), the module's data entries and `WriteData`
     (empty at first), and a backend skeleton with frame layout, prologue/epilogue and `ret`. Then
     integer operators, assignment and locals.
  2. Control flow: `if`, loops, `break`/`continue`, `&&`, `||`, `?:`, `switch`.
  3. Calls and ABI: the `call` instruction, the classifier interface, `abi.cpp` moves behind it
     (scalars only), parameters, more than 8 arguments, recursion.
  4. Arrays, pointers, globals: `addr`, `load`/`store` by type, pointer scaling, global and `static`
     data entries from `ConstValue` (21.2a), `enum`, string entries, `char`, `sizeof`.
  5. Floats: `F32`/`F64`, conversions, float-constant entries, the floating rows of the classifier.
  6. Structs and unions: `memcpy`/`memzero`, struct arguments and returns (`Fields`, by-reference,
     hidden pointer), local initialisers.
  7. Delete the old path: make `--ir` the default and require 283/283 on it, then remove `EmitRISC`,
     the RISC-V parts of `emit.cpp` and `Context`, and the flag. Rewrite the CLAUDE.md "Architecture"
     section and design sections 3 to 6 to describe the IR.
- Each step adds `tests/custom/` cases (D13) only where the IR introduces a behaviour that the
  current tests do not cover, for example a temp that is live across a call inside a loop.

## 21.8 What this does not do

- No optimisation, no register allocator, no SSA. The output is expected to be slower than today's
  output, not faster.
- No second backend. v3 ends when RISC-V runs only through the IR. Whether a second target is worth
  building is a separate decision.
- The output is not byte-identical to v2. The 283 tests check behaviour, not assembly text.

## 21.9 Resolved in review (Marcus, 2026-11-11)

1. How `--ir` reaches the runner: `COMPILER_FLAGS` (and `TARGET`) in `run_tests.sh`, forwarded into
   Docker (21.7).
2. `Target()` is a global set in `main` before parsing. Using it before it is set aborts with an
   error and never falls back to a default (21.3).
3. One slot per temporary, with no reuse. Slot reuse is noted as a possible later optimisation (21.4).
4. Added: `--emit-ir` and the IR checker, both in step 1 (21.2). The checker runs on every `--ir`
   compile, in every build, not only in `DEBUG` builds (Marcus, 2026-11-11). With `--emit-ir` the
   dump is written before the checker runs, and the checker's errors follow it (Marcus, 2026-11-11,
   recorded in step 1).
5. Added: module-level data (globals from `ConstValue`, strings, float constants) as IR entries that
   the backend writes to `.data`/`.rodata` (21.2a).

## 21.10 Amendments (Marcus, 2026-11-11, D23)

1. **Pointer arithmetic.** New instruction `ptradd ptr, offset` (Arithmetic group): `ptr` is `Ptr`,
   `offset` is the pointer-width integer type (`I32` on RV32), a byte count, and the result is `Ptr`.
   The frontend multiplies the index by the element size and widens it to pointer width first, with
   `sext` or `zext` chosen by the index's C type. New conversions `ptrtoint` (`Ptr` to the
   pointer-width integer) and `inttoptr` (the reverse). Pointer difference is `ptrtoint` of both
   operands, `sub`, then `div` by the element size. The checker accepts `Ptr` only in these
   instructions, compares, addresses, `copy`, `load`/`store` values and `ret`. Implemented in step 4.
2. **Immediates.** Any integer value is a legal IR immediate. Each backend materialises one into a
   scratch register when it does not fit the instruction's encoding.
3. **Partial dump.** If lowering throws, `--emit-ir` writes the finished functions and the partly
   built one, then reports the error and fails.
