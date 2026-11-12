# Random ABI check against GCC

`gen_abi.py SEED COUNT DIR` writes `callee.c`, `caller.c` and `driver.c` for COUNT random
signatures (0 to 14 parameters for a uniform draw, up to 20 for the shaped ones below, from int, unsigned, the char and short types, `int *`, `char *`, float, double;
return type from the same set plus void). The callee checks every parameter and returns a value
derived from them; the caller checks it. Mismatches call `abi_fail` in the driver (always GCC).

`run_abi.sh SEED COUNT [GCC_OPT...]` builds caller and callee with ours (`build/c_compiler --ir`,
override with `OURS_FLAGS`) and with GCC at each option (default `-O0 -O2`; `-O2` is always run, added if you pass other options, never twice), links all four
combinations (ours->ours, ours->gcc, gcc->ours, gcc->gcc) and runs them under `spike pk`.
`-O2` is mandatory on the GCC side: `-O0` GCC re-extends narrow values and hides missing extensions.

Run from the repo root (output goes to `tests/abi_check/out/`, delete it afterwards):

    docker run --rm --entrypoint "" -v "$PWD":/w -w /w agent-compiler:latest bash -c \
      'export PATH=$PATH:/opt/riscv/bin; make build/c_compiler && \
       for s in 101 202 303; do tests/abi_check/run_abi.sh $s 300; done'

Float and double are in `PARAM_TYPES` since step 5. A uniform draw almost never uses up fa0-fa7, so
half the signatures are shaped (`gen_params`): `fp_heavy` (9 to 12 float/double parameters among
others: floats and doubles in a registers, doubles in a register pair and on the stack) and `split`
(8 float/double and 7 other parameters, then a double split between a7 and the stack). A split double
is checked in the callee byte by byte through an `unsigned char *`, never read as a double: GCC's
callee (-O0 and -O2) spills a7 next to the caller's first stack word and reads the double with an
`fld` from an address that is 4 mod 8, and `spike pk` emulates that misaligned `fld` wrongly (GCC to
GCC fails too). GCC's layout itself follows the psABI. Step 6 should add struct types.
