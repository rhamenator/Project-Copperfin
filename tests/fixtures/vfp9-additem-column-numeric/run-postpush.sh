#!/usr/bin/env bash
# RQ-CF-PRG-ADDITEM-COLUMN-NUMERIC-001 / VR001/002 exact signed-head recheck.
set -u
cd /home/rich/.codex/worktrees/additem-column-numeric-5611/Project-Copperfin || exit 2
expected_head="$1"
[[ "$expected_head" =~ ^[a-f0-9]{40}$ ]] || exit 3
evidence=build/additem-column-native
git rev-parse HEAD > "$evidence/postpush-head-before.txt"
[ "$(< "$evidence/postpush-head-before.txt")" = "$expected_head" ] || exit 4
sha256sum -c "$evidence/fixed-inputs.sha256" > "$evidence/postpush-inputs-before.log" || exit 5
sha256sum -c "$evidence/initial-five.sha256" > "$evidence/postpush-initial-five-before.log" || exit 6
regex='^(test_native_test_isolation_contract|test_prg_engine_((additem_column|additem|addlistitem|removelistitem|removeitem|topitemid|topindex|listitemid|listindex|list_index|listitem|itemidtoindex|indextoitemid|selectedid|selected_index|itemdata_index|getcolor_default)_numeric(_.*)?|numeric_behavior|int64_exact_expression|runtime_surface_functions|runtime_surface_itemid_selectors))$'
ctest --test-dir build/additem-column-gcc -N -R "$regex" > "$evidence/postpush-test-list.log" 2>&1
rg -q 'Total Tests: 90$' "$evidence/postpush-test-list.log" || exit 7
export TMPDIR=/home/rich/.codex/worktrees/additem-column-numeric-5611/Project-Copperfin/build/additem-column-native/tmp
nice -n 10 ionice -c 2 -n 7 ctest --test-dir build/additem-column-gcc -V -R "$regex" --parallel 1 --timeout 700 > "$evidence/postpush-gcc.log" 2>&1
result=$?
printf '%s\n' "$result" > "$evidence/postpush-gcc.status"
date -u +%Y-%m-%dT%H:%M:%SZ > "$evidence/postpush-gcc.completed-at"
sha256sum "$evidence/postpush-gcc.log" > "$evidence/postpush-gcc.log.sha256"
sha256sum -c "$evidence/fixed-inputs.sha256" > "$evidence/postpush-inputs-after.log" || exit 8
sha256sum -c "$evidence/initial-five.sha256" > "$evidence/postpush-initial-five-after.log" || exit 9
git rev-parse HEAD > "$evidence/postpush-head-after.txt"
[ "$(< "$evidence/postpush-head-after.txt")" = "$expected_head" ] || exit 10
exit "$result"
