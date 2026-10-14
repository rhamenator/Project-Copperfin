#!/usr/bin/env bash
# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only
# Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
# VR-5611-ADDLISTITEM-COLUMN-NUMERIC-002: comment-only notice phase.
set -u
cd /home/rich/.codex/worktrees/addlistitem-column-numeric-5611/Project-Copperfin || exit 2
evidence=build/addlistitem-column-native
fixture=tests/fixtures/vfp9-addlistitem-column-numeric
[ "$(< "$evidence/fixed-complete-audit.status")" = 0 ] || exit 3
node - "$fixture" "$evidence" <<'JS' || exit 4
const fs=require('fs'),assert=require('assert'),crypto=require('crypto');
const [f,e]=process.argv.slice(2),file=f+'/excluded-scope-preservation.cpp';
const original=fs.readFileSync(f+'/excluded-scope-preservation.before-license.txt');
const first=fs.readFileSync(e+'/preservation-source-frozen.sha256','utf8').split('\n')[0];
assert.strictEqual(crypto.createHash('sha256').update(original).digest('hex'),first.split('  ')[0]);
assert.strictEqual(fs.readFileSync(file,'utf8').replace('// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.\n',''),original.toString('utf8'));
console.log('Exact historical source preserved; only exception notice added.');
JS
sha256sum -c "$evidence/fixed-inputs.sha256" > "$evidence/notice-fixed259-before.log" || exit 5
sha256sum "$fixture/excluded-scope-preservation.cpp" "$fixture/excluded-scope-preservation.before-license.txt" "$fixture/run-preservation-notice.sh" tests/prg_engine_test_support.cpp > "$evidence/preservation-notice-inputs.sha256"
export ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=print_stacktrace=1 COPPERFIN_LOCALE=en-US
for name in gcc clang; do
 compiler=/usr/bin/c++
 flags=(-std=c++20 -O1 -g0)
 if [ "$name" = clang ]; then
  compiler=/usr/bin/clang++
  flags+=(-fsanitize=address,undefined,float-cast-overflow -fno-omit-frame-pointer)
 fi
 prefix="$evidence/preservation-notice-$name"
 mkdir -p "$prefix-tmp"
 export TMPDIR="/home/rich/.codex/worktrees/addlistitem-column-numeric-5611/Project-Copperfin/$prefix-tmp"
 mapfile -t libraries < <(rg --files "build/addlistitem-column-$name" -g '*.a' | sort)
 [ "${#libraries[@]}" = 9 ] || exit 6
 sha256sum "${libraries[@]}" > "$prefix-libraries.sha256"
 nice -n 10 ionice -c 2 -n 7 "$compiler" "${flags[@]}" -Iinclude -Itests -Isrc/runtime "$fixture/excluded-scope-preservation.cpp" tests/prg_engine_test_support.cpp -Wl,--start-group "${libraries[@]}" -Wl,--end-group -pthread -ldl -lz -o "$prefix" > "$prefix-compile.log" 2>&1
 result=$?
 printf '%s\n' "$result" > "$prefix-compile.status"
 [ "$result" = 0 ] || exit 7
 nice -n 10 ionice -c 2 -n 7 "$prefix" "/home/rich/.codex/worktrees/addlistitem-column-numeric-5611/Project-Copperfin/$prefix-scripts" > "$prefix.out" 2> "$prefix.stderr"
 result=$?
 printf '%s\n' "$result" > "$prefix.status"
 [ "$result" = 0 ] && [ ! -s "$prefix.stderr" ] || exit 8
 cmp "$evidence/preservation-fixed-$name.out" "$prefix.out" > "$prefix.cmp.log" 2>&1
 result=$?
 printf '%s\n' "$result" > "$prefix.cmp.status"
 [ "$result" = 0 ] || exit 9
 node "$fixture/audit-preservation.cjs" "$prefix.out" > "$prefix-audit.jsonl" 2> "$prefix-audit.stderr"
 result=$?
 printf '%s\n' "$result" > "$prefix-audit.status"
 [ "$result" = 0 ] || exit 10
 sha256sum -c "$prefix-libraries.sha256" > "$prefix-libraries-after.log" || exit 11
 sha256sum "$prefix.out" > "$prefix.out.sha256"
done
sha256sum -c "$evidence/preservation-notice-inputs.sha256" > "$evidence/preservation-notice-inputs-after.log" || exit 12
sha256sum -c "$evidence/fixed-inputs.sha256" > "$evidence/notice-fixed259-after.log" || exit 13
