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

## AAPCS64 model (aarch64, steps A3 and A5)

`gen_abi.py ... aarch64` draws only what the compiler supports: integer, pointer, float and double parameters and results
(`int`, `unsigned`, the char and short types, `int *`, `char *`, `float`, `double`; void result), no records (A6 extends
the model, so the C files carry no struct declarations). Pointers are 8 bytes, plain `char` is unsigned. Shapes:
`random` (0 to 14 parameters), `stack` (9 to 20), `x_full` (8 integers or pointers plus 1 to 4 of any type),
`narrow_tail` (8 then 1 to 8 char/short parameters, so narrow values sit in stack slots), `ptr_tail` (8 integers, then
pointers), `fp_heavy` (9 to 14 floats and doubles among 0 to 6 integers), `v_full` (8 floats and doubles plus 1 to 4 of
any type) and `both_full` (8 of each class in any order, then 1 to 6 of any type).

`classify_aapcs64` is written from the AAPCS64 text (section 6.8.2 parameter passing, 6.9 result return), independent
of `src/call_abi_aarch64.cpp`. It is the stage-C state machine with NGRN, NSRN and NSAA (stage A: 0, 0, SP), in
`aapcs64_place`, one argument at a time, so that A6 adds rules to it. It implements stage B (scalars are unmodified),
C.1 (a float or double goes in v[NSRN] if NSRN < 8), C.5 and C.6 (otherwise a float's size is set to 8, and the value is
copied at NSAA, which advances by the size), C.9 (an integral or pointer argument of up to 8 bytes goes in x[NGRN] if
NGRN < 8), and C.14, C.16, C.17 (otherwise NSAA is rounded up to max(8, alignment), a size under 8 is set to 8, the
argument is copied at NSAA and NSAA advances by the size), plus the rule that SP is 16-byte aligned at a call. A result
(6.9) is placed by the same machine as the argument of `void f(T)`: x0/w0 or v0 (s0, d0). The floating rules (A5) were
written from the standard before the compiler's floating classifier rows existed. The rule numbers are those of the
current ARM-software/abi-aa `aapcs64.rst` (checked 2026-11-19), where C.7 and C.8 are the SVE rules.

Its cases, with the same hit table and `every case hit` / `NOT HIT:` last line as ilp32d:
`x_int`, `x_ptr`, `x_narrow` (in an x register), `x_last` (the eighth, in x7), `x_after_v` (in an x register once
v0-v7 are used up), `v_float`, `v_double` (in a v register), `v_last` (the eighth, in v7), `v_after_x` (in a v register
once x0-x7 are used up), `s_first` (the first stack argument), `s_int`, `s_ptr`, `s_narrow`, `s_float`, `s_double` (on
the stack), `s_int_vfree` and `s_fp_xfree` (a stack argument of one class while the other class still has free
registers: the counts are independent), `s_mixed` (a stack argument of the other class than the one before it: one NSAA
serves both), `s_next` (a stack argument at NSAA > 0), `s_odd` (an odd number of stack arguments: the area is padded to
16), `r_int`, `r_ptr`, `r_narrow`, `r_float`, `r_double` (results). Nothing is split between registers and the stack on
aarch64, and the byte-wise check of a split double is ilp32d only.

    docker run --rm --entrypoint "" --user "$(id -u):$(id -g)" -v "$PWD":/w -w /w agent-compiler:latest bash -c \
      'make build/c_compiler && for s in 101 202 303 404 505; do TARGET=aarch64 tests/abi_check/run_abi.sh $s 300; done'

Seeds 101, 202, 303, 404, 505 x 300 hit every aarch64 case at least 13 times per seed (`s_mixed` is the rarest) and pass in
all four combinations at GCC -O0 and -O2 under `qemu-aarch64` (step A5).
