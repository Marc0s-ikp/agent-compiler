Custom tests that run only under `TARGET=riscv32` (`scripts/run_tests.sh` finds them only for that target and skips one named
under another). Same format as `tests/custom/`: `<name>.c`, `<name>_driver.c`, and optionally `<name>.gccflags` with extra
GCC flags for the driver. Put a test here only when it cannot be written for both targets (for example an 8-byte literal).
