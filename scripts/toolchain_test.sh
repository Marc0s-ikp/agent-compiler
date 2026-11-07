#!/bin/bash

# Author : James Nock (@Jpnock)
# Year   : 2023
# Modified to run all tests recursively and continue on failures

set -euo pipefail

# Build the compiler once
make build/c_compiler

mkdir -p build

total=0
passed=0

# Use process substitution to avoid subshell and preserve counters
while IFS= read -r -d '' test_file; do
    total=$((total + 1))

    # Get the directory and base name
    test_dir=$(dirname "$test_file")
    base_name=$(basename "$test_file" .c)
    driver_file="${test_dir}/${base_name}_driver.c"

    # Check if driver exists
    if [ ! -f "$driver_file" ]; then
        echo "ERROR: Driver not found for $test_file"
        continue
    fi

    # Create unique names for assembly and executable
    safe_name=$(echo "$test_file" | sed 's/[\/.]/_/g')
    asm_file="build/${safe_name}.s"
    elf_file="build/${safe_name}.elf"

    echo "========================================"
    echo "Running test: $test_file"

    # Compile with compiler
    if ! ./build/c_compiler -S "$test_file" -o "$asm_file"; then
        echo "❌ Compilation failed for $test_file"
        continue
    fi

    # Link with driver using RISC-V GCC
    if ! riscv64-unknown-elf-gcc -march=rv32imfd -mabi=ilp32d -o "$elf_file" "$asm_file" "$driver_file"; then
        echo "❌ Linking failed for $test_file"
        # Keep assembly for debugging
        continue
    fi

    # Run with spike
    if spike pk "$elf_file" > /dev/null 2>&1; then
        echo "Test passed: $test_file"
        passed=$((passed + 1))
        # Clean up on success
        rm -f "$asm_file" "$elf_file"
    else
        echo "❌ test failed at runtime: $test_file"
        # Keep assembly and executable for debugging
    fi

done < <(find tests -type f -name "*.c" ! -name "*_driver.c" -print0)

echo "========================================"
echo "Summary: $passed / $total tests passed"
if [ $passed -eq $total ]; then
    echo "All tests passed!"
    exit 0
else
    echo "Some tests failed."
    exit 1
fi
