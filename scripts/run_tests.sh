#!/usr/bin/env bash
# Runs the compiler against every test (or the ones given as arguments).
# Each test is <name>.c plus <name>_driver.c; the driver returns 0 on success.
# Output: results/latest.txt and a summary line on stdout.
set -u

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
if ! command -v spike >/dev/null 2>&1 || ! command -v riscv64-unknown-elf-gcc >/dev/null 2>&1; then
  exec docker run --rm --user "$(id -u):$(id -g)" -v "$ROOT":/w -w /w agent-compiler ./scripts/run_tests.sh "$@"
fi

GCC="${RISCV_GCC:-riscv64-unknown-elf-gcc}"
ARCH_FLAGS="-march=rv32imfd -mabi=ilp32d"
SIM="${RISCV_SIM:-spike pk}"
TIMEOUT="${TEST_TIMEOUT:-5}"
OUT="$ROOT/results/latest.txt"
WORK="$ROOT/build/test_out"

make -s build/c_compiler || { echo "BUILD FAILED"; exit 1; }
mkdir -p "$WORK" "$ROOT/results"
: > "$OUT"

if [ "$#" -gt 0 ]; then
  TESTS=("$@")
else
  mapfile -t TESTS < <(find tests/compiler_tests tests/custom -name '*.c' ! -name '*_driver.c' | sort)
fi

pass=0; total=0
for t in "${TESTS[@]}"; do
  total=$((total + 1))
  name="${t%.c}"
  rel="${name#tests/}"
  base="$WORK/${rel//\//_}"
  driver="${name}_driver.c"

  if ! timeout "$TIMEOUT" build/c_compiler -S "$t" -o "$base.s" > "$base.compile.log" 2>&1; then
    echo "CRASH   $rel (compiler)" >> "$OUT"; continue
  fi
  if ! $GCC $ARCH_FLAGS -o "$base.elf" "$base.s" "$driver" > "$base.link.log" 2>&1; then
    echo "FAIL    $rel (assemble/link)" >> "$OUT"; continue
  fi
  timeout "$TIMEOUT" $SIM "$base.elf" > "$base.run.log" 2>&1
  rc=$?
  if [ "$rc" -eq 0 ]; then
    echo "PASS    $rel" >> "$OUT"; pass=$((pass + 1))
  elif [ "$rc" -eq 124 ]; then
    echo "TIMEOUT $rel" >> "$OUT"
  else
    echo "FAIL    $rel (exit $rc)" >> "$OUT"
  fi
done

echo "$pass/$total passing" | tee -a "$OUT"
