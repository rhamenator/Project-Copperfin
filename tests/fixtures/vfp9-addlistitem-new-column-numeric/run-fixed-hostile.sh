#!/usr/bin/env bash
# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only
# Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
# RQ-CF-PRG-ADDLISTITEM-NEW-COLUMN-NUMERIC-001 / VR001+002.
# Execute PRE-WIRE-authored112 hostile cases/shard ONLY AFTER BOTH fixed100.
# Never an original-public hostile qualification or native exact64/NaN oracle.
set -u
cd /home/rich/.codex/worktrees/addlistitem-new-column-numeric-5611/Project-Copperfin || exit 2
fixture=tests/fixtures/vfp9-addlistitem-new-column-numeric
evidence=build/addlistitem-new-column-native
[ "$(< "$evidence/fixed-driver.status")" = 0 ] || exit 3
[ "$(< "$evidence/fixed/complete-audit.status")" = 0 ] || exit 3
export ASAN_OPTIONS=detect_leaks=0
export UBSAN_OPTIONS=print_stacktrace=1
export COPPERFIN_LOCALE=en-US
for compiler in gcc clang; do
 mkdir -p "$evidence/hostile-$compiler-tmp"
 chmod 700 "$evidence/hostile-$compiler-tmp"
 export TMPDIR=/home/rich/.codex/worktrees/addlistitem-new-column-numeric-5611/Project-Copperfin/$evidence/hostile-$compiler-tmp
 binary="build/addlistitem-new-column-$compiler/tests/test_prg_engine_addlistitem_new_column_numeric"
 sha256sum -c "$fixture/fixed-inputs.sha256" > "$evidence/hostile-$compiler-inputs-before.log" || exit 4
 sha256sum "$binary" > "$evidence/hostile-$compiler-binary.sha256"
 for mode in COPPERFIN VFP9; do
  for session in 1 2; do
   nice -n 10 ionice -c 2 -n 7 "$binary" "$mode" "$session" --fixed-only-hostile > "$evidence/hostile-$compiler-$mode-$session.log" 2> "$evidence/hostile-$compiler-$mode-$session.stderr"
   result=$?
   printf '%s\n' "$result" > "$evidence/hostile-$compiler-$mode-$session.status"
   [ "$result" = 0 ] || exit 5
   [ ! -s "$evidence/hostile-$compiler-$mode-$session.stderr" ] || exit 5
   [ "$(< "$evidence/hostile-$compiler-$mode-$session.log")" = 'AddListItem NEW COLUMN frozen cases: direct=0 public=112' ] || exit 6
  done
 done
 sha256sum -c "$evidence/hostile-$compiler-binary.sha256" > "$evidence/hostile-$compiler-binary-after.log" || exit 7
 sha256sum -c "$fixture/fixed-inputs.sha256" > "$evidence/hostile-$compiler-inputs-after.log" || exit 7
 date -u +%Y-%m-%dT%H:%M:%SZ > "$evidence/hostile-$compiler.completed-at"
 printf '%s all four fixed-only112 shards completed0; full stdout counters/empty stderr/source270/binary hashes checked.\n' "$compiler"
done
