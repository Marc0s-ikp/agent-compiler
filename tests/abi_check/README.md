# Random ABI check against GCC

`gen_abi.py SEED COUNT DIR [TARGET]` (TARGET `riscv32` by default, or `aarch64`; `run_abi.sh` passes it) writes `callee.c`, `caller.c` and `driver.c` for COUNT random
signatures (0 to 14 parameters for a uniform draw, up to 20 for the shaped ones below, from int, unsigned, the char and short types, `int *`, `char *`, float, double;
return type from the same set plus void). The callee checks every parameter and returns a value
derived from them; the caller checks it. Mismatches call `abi_fail` in the driver (always GCC).

`run_abi.sh SEED COUNT [GCC_OPT...]` builds caller and callee with ours (`build/c_compiler`,
extra flags via `OURS_FLAGS`) and with GCC at each option (default `-O0 -O2`; `-O2` is always run, added if you pass other options, never twice), links all four
combinations (ours->ours, ours->gcc, gcc->ours, gcc->gcc) and runs them under `spike pk`.
`TARGET` (default `riscv32`) selects the target as in `scripts/run_tests.sh`: it is passed to ours as
`--target`, and `TARGET=aarch64` builds with `aarch64-linux-gnu-gcc`, links with `-static` and runs under
`qemu-aarch64`. The generator draws and models by target: `classify` (ilp32d) for riscv32, `classify_aapcs64` for aarch64
(see "AAPCS64 model" below). The riscv32 output for a seed is the same as before aarch64 existed.
`-O2` is mandatory on the GCC side: `-O0` GCC re-extends narrow values and hides missing extensions.

Run from the repo root (output goes to `tests/abi_check/out/`, delete it afterwards):

    docker run --rm --entrypoint "" --user "$(id -u):$(id -g)" -v "$PWD":/w -w /w agent-compiler:latest bash -c \
      'export PATH=$PATH:/opt/riscv/bin; make build/c_compiler && \
       for s in 101 202 303; do tests/abi_check/run_abi.sh $s 300; done'

Float and double are in `PARAM_TYPES` since step 5. A uniform draw almost never uses up fa0-fa7, so
half the signatures are shaped (`gen_params`): `fp_heavy` (9 to 12 float/double parameters among
others: floats and doubles in a registers, doubles in a register pair and on the stack) and `split`
(8 float/double and 7 other parameters, then a double split between a7 and the stack). A split double
is checked in the callee byte by byte through an `unsigned char *`, never read as a double: GCC's
callee (-O0 and -O2) spills a7 next to the caller's first stack word and reads the double with an
`fld` from an address that is 4 mod 8, and `spike pk` emulates that misaligned `fld` wrongly (GCC to
GCC fails too). GCC's layout itself follows the psABI.

Struct and union types are in `PARAM_TYPES` since step 6. `RECORDS` in `gen_abi.py` is the one table: each
record is its fields (scalars, arrays, earlier records), and its declaration, checked leaves, size and alignment
are derived from it. It has integer-only structs of 1 to 16 bytes, float/double structs (also as `float[2]` and a
nested struct), float+int mixes in both orders (with `short`, `signed char`, `double`), structs that are not in
the floating convention although they hold a float (`{float, int *}`, three floats, a union inside), and three
unions with a float or double member. Each leaf is checked in the callee and, for a struct result, in the caller.

`classify` is a small model of the ilp32d rules (independent of the compiler: flattening, the fa/a register
counts, the a7 split, the stack, by reference, the hidden result pointer). It does two things:
- It finds the parameters split between a7 and the stack (a double, or a struct or union holding one); only their
  doubles are compared by bytes (see above). If the model missed a split, GCC to GCC would fail under pk.
- It counts, for each run, how many signatures (and arguments or results) hit each ABI case, and prints the table
  (also `out/seedN/hits.txt`). A case with no hits is not tested; the last line says `every case hit` or lists
  the missing ones. Cases: (a) struct <= 4 bytes in one a register, (b) 5-8 bytes in two, (c) split across a7 and
  the stack, (d) small struct wholly on the stack, (e) float+float in two fa, (f) float+int in fa + a,
  (g) one float/double field in one fa, (h) floating-convention struct that falls back to the integer convention
  because fa or a ran out, (i_reg / i_stack) over 8 bytes by reference with the pointer in a register / on the
  stack, (j) return through the hidden pointer, (k) small return in a0/a1, (l) floating return in fa0/fa1,
  (m) union holding floats, (n) struct argument once fa0-fa7 are used up, (x_dsplit) a scalar double split,
  (x_pad8) an 8-aligned value on the stack after a padding word.

Shapes beside `random`, `fp_heavy` and `split`: `structs` (1 to 6 records among 0 to 9 scalars), `fa_full`
(fa0-fa7 used up by doubles and floats, then records), `struct_split` (a0-a6, or a1-a6 with a hidden result
pointer, used up, then a 5 to 8 byte record; half the time fa0-fa7 are full first, so a floating struct is split
too) and `a_full` (a0-a7 and sometimes the first stack word used, then records). Seeds 101, 202, 303, 404, 505
x 300 hit every case 4 to 130 times per seed and pass in all four combinations with GCC -O0 and -O2.
`run_abi.sh` deletes the ELFs of an earlier run of the same seed (it skips an ELF that exists).

## AAPCS64 model (aarch64, steps A3, A5 and A6)

`gen_abi.py ... aarch64` draws integer, pointer, float and double parameters and results and (since A6) structs and
unions. Pointers are 8 bytes, plain `char` is unsigned, and `layout()` (sizes, alignment) follows the target: riscv32 uses
the ILP32 scalars, aarch64 the LP64 ones. `RECORDS` is built per target by `setup_target`: the table every riscv32 file
has (`COMMON_RECORDS`, unchanged, so riscv32 output for a seed is byte-identical) plus `AARCH64_RECORDS`, which only aarch64
draws (4 floats as an array, 3 doubles, 4 doubles nested, a union of doubles, five floats, `union { float; double; }`,
char arrays of 7, 11, 13 bytes, `int[5]`, `{double, double, int}`, a union over 16 bytes, records with pointers). The
declaration, leaves, size, alignment and HFA-ness of each are derived from the one table.

The model is written from the AAPCS64 text, independent of `src/call_abi_aarch64.cpp` (rule numbers are those of the
current ARM-software/abi-aa `aapcs64.rst`, checked 2026-11-19). `aapcs64_place` applies the standard's rule list in
order and returns where the argument went (`Place`: v registers, x registers or a stack offset, and whether B.4 replaced it
by a pointer):
- Stage A: NGRN = NSRN = 0, NSAA = SP (`Aapcs64State`).
- Stage B (`aapcs64_stage_b`): B.3 an HFA is unmodified; B.4 a composite over 16 bytes becomes a pointer to a caller copy;
  B.5 any other composite's size is rounded up to a multiple of 8. B.1, B.2 and B.6 cannot apply to our types.
- Stage C: `c1` to `c17`, each one function with the standard's number and a one-line paraphrase. C.1 float in v[NSRN];
  C.2 an HFA in NSRN..NSRN+n-1 if NSRN + n <= 8; C.3 otherwise NSRN = 8 and the size is rounded to 8; C.4 NSAA rounded to 8;
  C.5 a float's size becomes 8; C.6 an HFA or float on the stack; C.9 integer or pointer in x[NGRN]; C.12 a composite in
  consecutive x registers if its double-words fit in 8 - NGRN; C.13 otherwise NGRN = 8 (a record is never split, and later
  integers go on the stack too); C.14 NSAA rounded to max(8, alignment); C.15 a composite on the stack; C.16 a size under 8
  becomes 8; C.17 the rest on the stack. C.7, C.8, C.10 and C.11 (scalable vectors, alignment 16, 16-byte integers) are
  functions that only assert they do not apply.
- Homogeneity (`hfa_of`): after layout every Fundamental Data Type is the same floating type and there are 1 to 4 uniquely
  addressable members. Members of a struct add up, members of a union overlap: a union counts its largest member.
  Checked against GCC (2026-11-19): `union { float a; float b; }` is 1 member (s0), `union { float a[2]; float b; }` 2,
  `union { float a[2]; float b[3]; }` 3 (s0 and s2 are read), `union { float; double; }` is not an HFA, and a struct holding
  two such unions counts their sum (4 members).
- The result (6.9, `aapcs64_result`) is derived, not tabulated: stage A, B, C on `void f(T)` with a fresh state; if the
  argument is passed as a value in registers, the result comes back in those registers (w0/x0, x0/x1, s0/d0, v0-v3);
  otherwise (B.4 made it a pointer) the result is in memory and its address is in x8. x8 is not an argument register.

Shapes (A3, A5): `random` (0 to 14 parameters), `stack` (9 to 20), `x_full` (8 integers or pointers plus 1 to 4 of any type),
`narrow_tail`, `ptr_tail`, `fp_heavy`, `v_full`, `both_full` (scalars only). A6 adds, with records: `records` (1 to 6 records
among scalars), `hfa_tail` (4 to 8 floats/doubles, then HFAs too big for the v registers left, then floats and doubles that
fitted before C.3, then records), `x7_left` (7 integers/pointers, then a 9-16 byte non-HFA record: NGRN 7, C.13; then
integers), `x_full_rec` (8 integers/pointers, then records), `x8_full` (8 integers/pointers, 0 to 2 records, and a result
in memory, so x0-x7 are full when x8 is used). Half of the results are records.

Cases (the table is printed per run with a count of signatures and occurrences; the last line is `every case hit` or
`NOT HIT:` and the missing names). Scalars: `x_int`, `x_ptr`, `x_narrow`, `x_last`, `x_after_v`, `v_float`, `v_double`,
`v_last`, `v_after_x`, `s_first`, `s_int`, `s_ptr`, `s_narrow`, `s_float`, `s_double`, `s_int_vfree`, `s_fp_xfree`, `s_mixed`
(a stack argument of another class than the one before it; records are a third class), `s_next`, `s_odd` (the stack area
is not a multiple of 16 before padding), `r_int`, `r_ptr`, `r_narrow`, `r_float`, `r_double`. HFAs in v registers (C.2): `h_v1`
to `h_v4` (members), `h_float`, `h_double`, `h_nested`, `h_array`, `h_union`, `h_big_v` (over 16 bytes by value: B.3 before B.4),
`h_last` (ending in v7). HFAs on the stack: `h_stack` (v registers used up), `h_stack_part` (NSRN < 8 but too few left: C.3),
`h_stack_fp` (a later float or double on the stack although v[NSRN] was free before C.3), `h_big_stack`. Records in x
registers (C.12): `c_x1`, `c_x2`, `c_x_odd` (size not 1, 2, 4, 8, 16), `c_x_float`, `c_x_union`, `c_x_last`. Records on the stack:
`c_stack_one` (9-16 bytes, one x register left: C.13, no split), `c_stack_int` (a later integer or pointer on the stack: x7
stays unused), `c_stack_full`, `c_stack_odd` (an odd-sized record followed by another stack argument: B.5). By reference
(B.4): `ref_x`, `ref_stack`. Results: `r_rec_x1` (x0), `r_rec_x2` (x0/x1), `r_hfa` (v registers), `r_hfa4` (v0-v3), `r_mem` (x8),
`r_mem_xfull` (x8 while x0-x7 are all used by arguments). Nothing is split between registers and the stack on aarch64; the
byte-wise check of a split double is ilp32d only.

`probe_aapcs64.py SEED COUNT DIR` checks the model itself against GCC, because GCC to GCC passing cannot show that the model is
right. A hand-written assembly callee `probe` records x0-x8, d0-d7 and 96 words from sp, and returns known patterns in x0, x1,
d0-d3 (and through [x8] when the model says memory). The caller GCC compiles at -O2 calls it through a cast to each generated
signature with arguments of known bytes; the script compares the registers, the stack words, the bytes behind a by-reference
pointer and the result with the placement the model predicts. Seeds 101 to 505 x 300 agree on every signature (a model with
C.3 removed disagrees on 15 of 300, one with C.13 removed crashes on a wrong pointer), and the first signature for each case is
printed with `ok`/`BAD`.

    docker run --rm --entrypoint "" --user "$(id -u):$(id -g)" -v "$PWD":/w -w /w agent-compiler:latest bash -c \
      'make build/c_compiler && for s in 101 202 303 404 505; do TARGET=aarch64 tests/abi_check/run_abi.sh $s 300; done'

Until the compiler passes records (A6), `run_abi.sh` on aarch64 stops at "ours failed"; the generator itself was checked with GCC
only (caller, callee and driver at -O0 and -O2 under `qemu-aarch64`): seeds 101, 202, 303, 404, 505 x 300 pass and hit every case.
