#!/usr/bin/env bash
set -u
cd /home/rich/.codex/worktrees/addlistitem-numeric-5611/Project-Copperfin || exit 2
evidence=build/addlistitem-native
mkdir -p "$evidence/tmp"
chmod 700 "$evidence/tmp"
export TMPDIR=/home/rich/.codex/worktrees/addlistitem-numeric-5611/Project-Copperfin/build/addlistitem-native/tmp
export ASAN_OPTIONS=detect_leaks=0
export UBSAN_OPTIONS=print_stacktrace=1
targets=(test_prg_engine_addlistitem_numeric test_prg_engine_removelistitem_numeric test_prg_engine_removeitem_numeric test_prg_engine_topitemid_numeric test_prg_engine_topindex_numeric test_prg_engine_listitemid_numeric test_prg_engine_listindex_numeric test_prg_engine_list_index_numeric test_prg_engine_listitem_numeric test_prg_engine_itemidtoindex_numeric test_prg_engine_indextoitemid_numeric test_prg_engine_selectedid_numeric test_prg_engine_selected_index_numeric test_prg_engine_itemdata_index_numeric test_prg_engine_getcolor_default_numeric test_prg_engine_numeric_behavior test_prg_engine_int64_exact_expression test_prg_engine_runtime_surface_functions test_prg_engine_runtime_surface_itemid_selectors)
regex='^(test_native_test_isolation_contract|test_prg_engine_((addlistitem|removelistitem|removeitem|topitemid|topindex|listitemid|listindex|list_index|listitem|itemidtoindex|indextoitemid|selectedid|selected_index|itemdata_index|getcolor_default)_numeric(_.*)?|numeric_behavior|int64_exact_expression|runtime_surface_functions|runtime_surface_itemid_selectors))$'
run_compiler() {
    local name="$1" compiler="$2" flags="$3"
    local build="build/addlistitem-$name"
    sha256sum -c "$evidence/refrozen-original-inputs.sha256" > "$evidence/refrozen-original-$name-inputs-before.log" || return 3
    nice -n 10 ionice -c 2 -n 7 cmake -S . -B "$build" -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER="$compiler" -DCMAKE_CXX_FLAGS_DEBUG="$flags" -DCOPPERFIN_BUILD_TESTS=ON > "$evidence/refrozen-configure-$name.log" 2>&1
    local result=$?
    printf '%s\n' "$result" > "$evidence/refrozen-configure-$name.status"
    [ "$result" = 0 ] || return 4
    nice -n 10 ionice -c 2 -n 7 cmake --build "$build" --parallel 1 --target "${targets[@]}" > "$evidence/refrozen-compile-$name.log" 2>&1
    result=$?
    printf '%s\n' "$result" > "$evidence/refrozen-compile-$name.status"
    [ "$result" = 0 ] || return 5
    ctest --test-dir "$build" -N -R "$regex" > "$evidence/refrozen-original-$name-test-list.log" 2>&1
    rg -q 'Total Tests: 80$' "$evidence/refrozen-original-$name-test-list.log" || return 6
    nice -n 10 ionice -c 2 -n 7 ctest --test-dir "$build" -V -R "$regex" --parallel 1 --timeout 700 > "$evidence/refrozen-original-$name.log" 2>&1
    result=$?
    printf '%s\n' "$result" > "$evidence/refrozen-original-$name.status"
    date -u +%Y-%m-%dT%H:%M:%SZ > "$evidence/refrozen-original-$name.completed-at"
    sha256sum "$evidence/refrozen-original-$name.log" > "$evidence/refrozen-original-$name.log.sha256"
    sha256sum -c "$evidence/refrozen-original-inputs.sha256" > "$evidence/refrozen-original-$name-inputs-after.log" || return 7
    printf '%s original completed with actual ctest status %s\n' "$name" "$result"
    return 0
}
run_compiler gcc /usr/bin/c++ '-O1 -g0' || exit "$?"
run_compiler clang /usr/bin/clang++ '-O1 -g0 -fsanitize=address,undefined,float-cast-overflow -fno-omit-frame-pointer' || exit "$?"
printf 'Both originals complete; production migration forbidden until full raw audit.\n'
