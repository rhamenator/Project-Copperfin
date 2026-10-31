#!/usr/bin/env bash
# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only
# Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
# RQ-CF-PRG-LISTITEM-COLUMN-NUMERIC-001 / VR001+002. Fixed106 only.
set -u
cd /home/rich/.codex/worktrees/listitem-column-read-5611/Project-Copperfin || exit 2
fixture=tests/fixtures/vfp9-listitem-column-numeric
evidence=build/listitem-column-native
mkdir -p "$evidence/fixed/tmp"; chmod 700 "$evidence/fixed/tmp"
[ "$(< "$evidence/originals-driver.status")" = 0 ] || exit 3
[ "$(< "$evidence/originals-complete-audit.status")" = 0 ] || exit 3
sha256sum -c "$fixture/fixed-inputs.sha256" > "$evidence/fixed/inputs-preflight.log" || exit 4
export TMPDIR=/home/rich/.codex/worktrees/listitem-column-read-5611/Project-Copperfin/build/listitem-column-native/fixed/tmp
export ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=print_stacktrace=1
targets=(test_prg_engine_listitem_column_numeric test_prg_engine_listitem_column_numeric_public test_prg_engine_addlistitem_column_numeric test_prg_engine_addlistitem_new_column_numeric test_prg_engine_additem_column_numeric test_prg_engine_additem_numeric test_prg_engine_addlistitem_numeric test_prg_engine_removelistitem_numeric test_prg_engine_removeitem_numeric test_prg_engine_topitemid_numeric test_prg_engine_topindex_numeric test_prg_engine_listitemid_numeric test_prg_engine_listindex_numeric test_prg_engine_list_index_numeric test_prg_engine_listitem_numeric test_prg_engine_itemidtoindex_numeric test_prg_engine_indextoitemid_numeric test_prg_engine_selectedid_numeric test_prg_engine_selected_index_numeric test_prg_engine_itemdata_index_numeric test_prg_engine_getcolor_default_numeric test_prg_engine_numeric_behavior test_prg_engine_int64_exact_expression test_prg_engine_runtime_surface_functions test_prg_engine_runtime_surface_itemid_selectors)
regex='^(test_native_test_isolation_contract|test_prg_engine_((listitem_column|addlistitem_column|addlistitem_new_column|additem_column|additem|addlistitem|removelistitem|removeitem|topitemid|topindex|listitemid|listindex|list_index|listitem|itemidtoindex|indextoitemid|selectedid|selected_index|itemdata_index|getcolor_default)_numeric(_.*)?|numeric_behavior|int64_exact_expression|runtime_surface_functions|runtime_surface_itemid_selectors))$'
run_compiler() {
 local name="$1" build="build/listitem-column-$1" result
 sha256sum -c "$fixture/fixed-inputs.sha256" > "$evidence/fixed/fixed-$name-inputs-before.log" || return 5
 nice -n 10 ionice -c 2 -n 7 cmake --build "$build" --parallel 1 --target "${targets[@]}" > "$evidence/fixed/fixed-compile-$name.log" 2>&1
 result=$?; printf '%s\n' "$result" > "$evidence/fixed/fixed-compile-$name.status"; [ "$result" = 0 ] || return 6
 ctest --test-dir "$build" -N -R "$regex" > "$evidence/fixed/fixed-$name-test-list.log" 2>&1 || return 7
 rg -q 'Total Tests: 106$' "$evidence/fixed/fixed-$name-test-list.log" || return 7
 ctest --test-dir "$build" --show-only=json-v1 -R "$regex" > "$evidence/fixed/fixed-$name-test-metadata.json" 2> "$evidence/fixed/fixed-$name-test-metadata.stderr" || return 7
 node "$fixture/audit-test-metadata.cjs" "$evidence/fixed/fixed-$name-test-metadata.json" > "$evidence/fixed/fixed-$name-metadata-audit.jsonl" 2> "$evidence/fixed/fixed-$name-metadata-audit.stderr" || return 7
 nice -n 10 ionice -c 2 -n 7 ctest --test-dir "$build" -V -R "$regex" --parallel 1 --timeout 700 > "$evidence/fixed/fixed-$name.log" 2>&1
 result=$?; printf '%s\n' "$result" > "$evidence/fixed/fixed-$name.status"
 date -u +%Y-%m-%dT%H:%M:%SZ > "$evidence/fixed/fixed-$name.completed-at"
 sha256sum "$evidence/fixed/fixed-$name.log" > "$evidence/fixed/fixed-$name.log.sha256"
 sha256sum -c "$fixture/fixed-inputs.sha256" > "$evidence/fixed/fixed-$name-inputs-after.log" || return 8
 printf '%s fixed106 actual CTest status %s\n' "$name" "$result"; return "$result"
}
run_compiler gcc; gcc_status=$?; run_compiler clang; clang_status=$?
[ "$gcc_status" = 0 ] && [ "$clang_status" = 0 ] || exit 9
printf 'Both fixed106 executions complete; full raw/counter/source audit remains required.\n'
