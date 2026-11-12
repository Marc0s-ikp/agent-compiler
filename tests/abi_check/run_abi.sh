#!/usr/bin/env bash
# Runs the random ABI check for every caller/callee combination.
# Must run where riscv64-unknown-elf-gcc and spike exist (the Docker image).
# Usage: tests/abi_check/run_abi.sh SEED COUNT [GCC_OPT...]
# Combos: ours->ours, ours->gcc, gcc->ours, gcc->gcc for each GCC_OPT (-O2 is always included; default -O0 -O2).
set -u
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
cd "$ROOT"
SEED=$1; COUNT=$2; shift 2
# GCC -O2 is mandatory (-O0 GCC re-extends narrow values and hides bugs):
# it is always in the list, once, even if the caller passes it too.
OPTS=("$@")
[ "$#" -eq 0 ] && OPTS=(-O0)
case " ${OPTS[*]} " in *" -O2 "*) ;; *) OPTS+=(-O2) ;; esac
GCC="riscv64-unknown-elf-gcc -march=rv32imfd -mabi=ilp32d"
OURS="build/c_compiler ${OURS_FLAGS:-}"
OUT="tests/abi_check/out/seed$SEED"
mkdir -p "$OUT"
# ELFs of an earlier run of this seed are stale (the loop below skips an existing ELF).
rm -f "$OUT"/*.elf
python3 tests/abi_check/gen_abi.py "$SEED" "$COUNT" "$OUT"
$OURS -S "$OUT/caller.c" -o "$OUT/caller_ours.s" || { echo "ours failed on caller.c"; exit 2; }
$OURS -S "$OUT/callee.c" -o "$OUT/callee_ours.s" || { echo "ours failed on callee.c"; exit 2; }
status=0
for opt in "${OPTS[@]}"; do
  $GCC $opt -S "$OUT/caller.c" -o "$OUT/caller_gcc$opt.s"
  $GCC $opt -S "$OUT/callee.c" -o "$OUT/callee_gcc$opt.s"
  for combo in "ours ours" "ours gcc$opt" "gcc$opt ours" "gcc$opt gcc$opt"; do
    set -- $combo
    elf="$OUT/$1_to_$2.elf"
    [ -f "$elf" ] && continue
    $GCC -O0 -o "$elf" "$OUT/caller_$1.s" "$OUT/callee_$2.s" "$OUT/driver.c" || { echo "link failed $combo"; status=1; continue; }
    result=$(timeout 60 spike pk "$elf" 2>&1); rc=$?
    nfail=$(grep -c '^FAIL' <<<"$result")
    echo "seed $SEED caller=$1 callee=$2: rc=$rc failures=$nfail $(grep '^abi:' <<<"$result")"
    grep '^FAIL' <<<"$result" | head -20
    [ "$rc" -ne 0 ] && status=1
  done
done
exit $status
