# Random ABI check against GCC

`gen_abi.py SEED COUNT DIR` writes `callee.c`, `caller.c` and `driver.c` for COUNT random
signatures (0 to 14 parameters from int, unsigned, the char and short types, `int *`, `char *`;
return type from the same set plus void). The callee checks every parameter and returns a value
derived from them; the caller checks it. Mismatches call `abi_fail` in the driver (always GCC).

`run_abi.sh SEED COUNT [GCC_OPT...]` builds caller and callee with ours (`build/c_compiler --ir`,
override with `OURS_FLAGS`) and with GCC at each option (default `-O0 -O2`), links all four
combinations (ours->ours, ours->gcc, gcc->ours, gcc->gcc) and runs them under `spike pk`.
Use `-O2` on the GCC side: `-O0` GCC re-extends narrow values and hides missing extensions.

Run from the repo root (output goes to `tests/abi_check/out/`, delete it afterwards):

    docker run --rm --entrypoint "" -v "$PWD":/w -w /w agent-compiler:latest bash -c \
      'export PATH=$PATH:/opt/riscv/bin; make build/c_compiler && \
       for s in 101 202 303; do tests/abi_check/run_abi.sh $s 300; done'

Steps 5 and 6 should add float/double and struct types to `PARAM_TYPES` once the IR path has them.
