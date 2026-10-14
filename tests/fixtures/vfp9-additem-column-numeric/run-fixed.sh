#!/usr/bin/env bash
# RQ-CF-PRG-ADDITEM-COLUMN-NUMERIC-001 / VR-5611-ADDITEM-COLUMN-NUMERIC-001/002.
# No frozen assertions/registrations altered; hostile corpus was authored pre-wire.
set -u
cd /home/rich/.codex/worktrees/additem-column-numeric-5611/Project-Copperfin || exit 2
evidence=build/additem-column-native
[ "$(< "$evidence/originals-complete-audit.status")" = 0 ] || exit 10
[ "$(< "$evidence/original-driver.status")" = 0 ] || exit 11
sha256sum -c "$evidence/initial-five.sha256" > "$evidence/fixed-initial-five-before.log" || exit 12
export TMPDIR=/home/rich/.codex/worktrees/additem-column-numeric-5611/Project-Copperfin/build/additem-column-native/tmp
export ASAN_OPTIONS=detect_leaks=0
export UBSAN_OPTIONS=print_stacktrace=1
targets=(test_prg_engine_additem_column_numeric test_prg_engine_additem_numeric test_prg_engine_addlistitem_numeric test_prg_engine_removelistitem_numeric test_prg_engine_removeitem_numeric test_prg_engine_topitemid_numeric test_prg_engine_topindex_numeric test_prg_engine_listitemid_numeric test_prg_engine_listindex_numeric test_prg_engine_list_index_numeric test_prg_engine_listitem_numeric test_prg_engine_itemidtoindex_numeric test_prg_engine_indextoitemid_numeric test_prg_engine_selectedid_numeric test_prg_engine_selected_index_numeric test_prg_engine_itemdata_index_numeric test_prg_engine_getcolor_default_numeric test_prg_engine_numeric_behavior test_prg_engine_int64_exact_expression test_prg_engine_runtime_surface_functions test_prg_engine_runtime_surface_itemid_selectors)
regex='^(test_native_test_isolation_contract|test_prg_engine_((additem_column|additem|addlistitem|removelistitem|removeitem|topitemid|topindex|listitemid|listindex|list_index|listitem|itemidtoindex|indextoitemid|selectedid|selected_index|itemdata_index|getcolor_default)_numeric(_.*)?|numeric_behavior|int64_exact_expression|runtime_surface_functions|runtime_surface_itemid_selectors))$'
run_compiler() {
    local name="$1" compiler="$2" flags="$3"
    local build="build/additem-column-$name"
    sha256sum -c "$evidence/fixed-inputs.sha256" > "$evidence/fixed-$name-inputs-before.log" || return 3
    nice -n 10 ionice -c 2 -n 7 cmake -S . -B "$build" -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER="$compiler" -DCMAKE_CXX_FLAGS_DEBUG="$flags" -DCOPPERFIN_BUILD_TESTS=ON > "$evidence/fixed-configure-$name.log" 2>&1
    local result=$?
    printf '%s\n' "$result" > "$evidence/fixed-configure-$name.status"
    [ "$result" = 0 ] || return 4
    nice -n 10 ionice -c 2 -n 7 cmake --build "$build" --parallel 1 --target "${targets[@]}" > "$evidence/fixed-compile-$name.log" 2>&1
    result=$?
    printf '%s\n' "$result" > "$evidence/fixed-compile-$name.status"
    [ "$result" = 0 ] || return 5
    ctest --test-dir "$build" -N -R "$regex" > "$evidence/fixed-$name-test-list.log" 2>&1
    rg -q 'Total Tests: 90$' "$evidence/fixed-$name-test-list.log" || return 6
    nice -n 10 ionice -c 2 -n 7 ctest --test-dir "$build" -V -R "$regex" --parallel 1 --timeout 700 > "$evidence/fixed-$name.log" 2>&1
    result=$?
    printf '%s\n' "$result" > "$evidence/fixed-$name.status"
    date -u +%Y-%m-%dT%H:%M:%SZ > "$evidence/fixed-$name.completed-at"
    sha256sum "$evidence/fixed-$name.log" > "$evidence/fixed-$name.log.sha256"
    sha256sum -c "$evidence/fixed-inputs.sha256" > "$evidence/fixed-$name-inputs-after.log" || return 7
    printf '%s fixed90 completed with actual ctest status %s\n' "$name" "$result"
    [ "$result" = 0 ] || return 8
    local mode selected
    for mode in COPPERFIN VFP9; do
      for selected in 1 2; do
        nice -n 10 ionice -c 2 -n 7 "$build/tests/test_prg_engine_additem_column_numeric" "$mode" "$selected" --fixed-only-hostile > "$evidence/hostile-fixed-$name-$mode-session$selected.log" 2>&1
        result=$?
        printf '%s\n' "$result" > "$evidence/hostile-fixed-$name-$mode-session$selected.status"
        sha256sum "$evidence/hostile-fixed-$name-$mode-session$selected.log" > "$evidence/hostile-fixed-$name-$mode-session$selected.log.sha256"
        [ "$result" = 0 ] || return 9
      done
    done
    sha256sum -c "$evidence/fixed-inputs.sha256" > "$evidence/hostile-fixed-$name-inputs-after.log" || return 13
    bash tests/fixtures/vfp9-additem-column-numeric/run-preservation.sh fixed "$name" > "$evidence/preservation-fixed-$name-driver.log" 2>&1
    result=$?
    printf '%s\n' "$result" > "$evidence/preservation-fixed-$name-driver.status"
    [ "$result" = 0 ] || return 14
    return 0
}
run_compiler gcc /usr/bin/c++ '-O1 -g0' || exit "$?"
run_compiler clang /usr/bin/clang++ '-O1 -g0 -fsanitize=address,undefined,float-cast-overflow -fno-omit-frame-pointer' || exit "$?"
printf 'Both complete fixed90, four542 hostile shards per compiler, and samecompiler296 raw comparisons finished; audit full statuses/counters/sources.\n'
