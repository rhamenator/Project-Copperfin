#!/usr/bin/env bash
# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only
# Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
# NON-NATIVE before/after capture only, never a requirement/compatibility oracle.
set -u
[ "$#" = 4 ] || exit 2
phase="$1"
compiler_name="$2"
product_root="$3"
evidence="$4"
[ "$phase" = baseline ] || [ "$phase" = fixed ] || exit 3
[ "$compiler_name" = gcc ] || [ "$compiler_name" = clang ] || exit 4
candidate_root=/home/rich/.codex/worktrees/listitem-column-read-5611/Project-Copperfin
fixture="$candidate_root/tests/fixtures/vfp9-listitem-column-numeric"
product_root=$(realpath "$product_root") || exit 5
mkdir -p "$evidence" || exit 5
evidence=$(realpath "$evidence") || exit 5
if [ "$phase" = baseline ]; then
 [ "$(git -C "$product_root" rev-parse HEAD)" = 1558300d4f113153b0406ba7191ba259b08ccbee ] || exit 6
else
 [ "$product_root" = "$candidate_root" ] || exit 6
fi
sha256sum -c "$fixture/preservation-inputs.sha256" > "$evidence/preservation-$phase-$compiler_name-inputs-before.log" || exit 7
mapfile -t libraries < <(rg --files "$product_root/build/listitem-column-$compiler_name" -g '*.a' | LC_ALL=C sort)
[ "${#libraries[@]}" = 9 ] || exit 8
sha256sum "${libraries[@]}" > "$evidence/preservation-$phase-$compiler_name-libraries.sha256"
compiler=/usr/bin/c++
flags=(-std=c++20 -O1 -g0)
if [ "$compiler_name" = clang ]; then
 compiler=/usr/bin/clang++
 flags+=(-fsanitize=address,undefined,float-cast-overflow -fno-omit-frame-pointer)
fi
tmp="$evidence/preservation-$phase-$compiler_name-tmp"
scripts="$evidence/preservation-$phase-$compiler_name-scripts"
mkdir -p "$tmp" "$scripts"; chmod 700 "$tmp" "$scripts"
export TMPDIR="$tmp" ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=print_stacktrace=1 COPPERFIN_LOCALE=en-US
binary="$evidence/preservation-$phase-$compiler_name"
nice -n 10 ionice -c 2 -n 7 "$compiler" "${flags[@]}" \
 -I"$product_root/include" -I"$product_root/tests" -I"$product_root/src/runtime" \
 "$fixture/excluded-scope-preservation.cpp" "$product_root/tests/prg_engine_test_support.cpp" \
 -Wl,--start-group "${libraries[@]}" -Wl,--end-group -pthread -ldl -lz \
 -o "$binary" > "$evidence/preservation-$phase-$compiler_name-compile.log" 2>&1
result=$?; printf '%s\n' "$result" > "$evidence/preservation-$phase-$compiler_name-compile.status"
[ "$result" = 0 ] || exit 9
nice -n 10 ionice -c 2 -n 7 "$binary" "$scripts" \
 > "$evidence/preservation-$phase-$compiler_name.out" \
 2> "$evidence/preservation-$phase-$compiler_name.stderr"
result=$?; printf '%s\n' "$result" > "$evidence/preservation-$phase-$compiler_name.status"
[ "$result" = 0 ] && [ ! -s "$evidence/preservation-$phase-$compiler_name.stderr" ] || exit 10
node "$fixture/audit-preservation.cjs" "$evidence/preservation-$phase-$compiler_name.out" \
 > "$evidence/preservation-$phase-$compiler_name-audit.jsonl" \
 2> "$evidence/preservation-$phase-$compiler_name-audit.stderr"
result=$?; printf '%s\n' "$result" > "$evidence/preservation-$phase-$compiler_name-audit.status"
[ "$result" = 0 ] || exit 11
sha256sum -c "$evidence/preservation-$phase-$compiler_name-libraries.sha256" \
 > "$evidence/preservation-$phase-$compiler_name-libraries-after.log" || exit 12
sha256sum -c "$fixture/preservation-inputs.sha256" \
 > "$evidence/preservation-$phase-$compiler_name-inputs-after.log" || exit 12
sha256sum "$evidence/preservation-$phase-$compiler_name.out" \
 > "$evidence/preservation-$phase-$compiler_name.out.sha256"
printf '%s\n' "$(git -C "$product_root" rev-parse HEAD)" \
 > "$evidence/preservation-$phase-$compiler_name.product-head"
date -u +%Y-%m-%dT%H:%M:%SZ > "$evidence/preservation-$phase-$compiler_name.completed-at"
if [ "$phase" = fixed ]; then
 cmp "$evidence/preservation-baseline-$compiler_name.out" \
     "$evidence/preservation-fixed-$compiler_name.out" \
     > "$evidence/preservation-fixed-$compiler_name.cmp.log" 2>&1
 result=$?; printf '%s\n' "$result" > "$evidence/preservation-fixed-$compiler_name.cmp.status"
 [ "$result" = 0 ] || exit 13
fi
