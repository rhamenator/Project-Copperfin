#!/usr/bin/env bash
# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only
# Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
# NON-NATIVE before/after capture only, never a requirement/compatibility oracle.
set -u
cd /home/rich/.codex/worktrees/addlistitem-new-column-numeric-5611/Project-Copperfin || exit 2
[ "$#" = 2 ] || exit 2
phase="$1"
name="$2"
[ "$phase" = baseline ] || [ "$phase" = fixed ] || exit 3
[ "$name" = gcc ] || [ "$name" = clang ] || exit 4
fixture=tests/fixtures/vfp9-addlistitem-new-column-numeric
evidence=build/addlistitem-new-column-native
# Do NOT race either original suite's SERIAL runtime execution or migrate code
# before BOTH original gates. Baseline capture follows those gates, precedes wire.
[ -f "$evidence/original-counter-v2/originals-complete-audit.status" ] || exit 5
[ "$(< "$evidence/original-counter-v2/originals-complete-audit.status")" = 0 ] || exit 5
if [ "$phase" = baseline ]; then
 sha256sum -c "$fixture/counter-v2-frozen-original-inputs.sha256" > "$evidence/preservation-$phase-$name-original270.log" || exit 6
 sha256sum -c "$fixture/unchanged-three-caller-inputs.sha256" > "$evidence/preservation-$phase-$name-callers3.log" || exit 6
fi
compiler=/usr/bin/c++
flags=(-std=c++20 -O1 -g0)
if [ "$name" = clang ]; then
 compiler=/usr/bin/clang++
 flags+=(-fsanitize=address,undefined,float-cast-overflow -fno-omit-frame-pointer)
fi
mkdir -p "$evidence/preservation-$phase-$name-tmp"
chmod 700 "$evidence/preservation-$phase-$name-tmp"
export TMPDIR=/home/rich/.codex/worktrees/addlistitem-new-column-numeric-5611/Project-Copperfin/$evidence/preservation-$phase-$name-tmp
export ASAN_OPTIONS=detect_leaks=0
export UBSAN_OPTIONS=print_stacktrace=1
export COPPERFIN_LOCALE=en-US
mapfile -t libraries < <(rg --files "build/addlistitem-new-column-$name" -g '*.a' | LC_ALL=C sort)
[ "${#libraries[@]}" = 9 ] || exit 7
sha256sum -c "$fixture/preservation-source-frozen.sha256" > "$evidence/preservation-$phase-$name-source-before.log" || exit 8
sha256sum "${libraries[@]}" > "$evidence/preservation-$phase-$name-libraries.sha256"
nice -n 10 ionice -c 2 -n 7 "$compiler" "${flags[@]}" -Iinclude -Itests -Isrc/runtime "$fixture/excluded-scope-preservation.cpp" tests/prg_engine_test_support.cpp -Wl,--start-group "${libraries[@]}" -Wl,--end-group -pthread -ldl -lz -o "$evidence/preservation-$phase-$name" > "$evidence/preservation-$phase-$name-compile.log" 2>&1
result=$?
printf '%s\n' "$result" > "$evidence/preservation-$phase-$name-compile.status"
[ "$result" = 0 ] || exit 9
nice -n 10 ionice -c 2 -n 7 "$evidence/preservation-$phase-$name" "/home/rich/.codex/worktrees/addlistitem-new-column-numeric-5611/Project-Copperfin/$evidence/preservation-$phase-$name-scripts" > "$evidence/preservation-$phase-$name.out" 2> "$evidence/preservation-$phase-$name.stderr"
result=$?
printf '%s\n' "$result" > "$evidence/preservation-$phase-$name.status"
sha256sum "$evidence/preservation-$phase-$name.out" > "$evidence/preservation-$phase-$name.out.sha256"
[ "$result" = 0 ] || exit 10
[ ! -s "$evidence/preservation-$phase-$name.stderr" ] || exit 10
sha256sum -c "$evidence/preservation-$phase-$name-libraries.sha256" > "$evidence/preservation-$phase-$name-libraries-after.log" || exit 11
sha256sum -c "$fixture/preservation-source-frozen.sha256" > "$evidence/preservation-$phase-$name-source-after.log" || exit 11
node "$fixture/audit-preservation.cjs" "$evidence/preservation-$phase-$name.out" > "$evidence/preservation-$phase-$name-audit.jsonl" 2> "$evidence/preservation-$phase-$name-audit.stderr"
result=$?
printf '%s\n' "$result" > "$evidence/preservation-$phase-$name-audit.status"
[ "$result" = 0 ] || exit 12
date -u +%Y-%m-%dT%H:%M:%SZ > "$evidence/preservation-$phase-$name.completed-at"
if [ "$phase" = fixed ]; then
 cmp "$evidence/preservation-baseline-$name.out" "$evidence/preservation-fixed-$name.out" > "$evidence/preservation-fixed-$name.cmp.log" 2>&1
 result=$?
 printf '%s\n' "$result" > "$evidence/preservation-fixed-$name.cmp.status"
 [ "$result" = 0 ] || exit 13
fi
