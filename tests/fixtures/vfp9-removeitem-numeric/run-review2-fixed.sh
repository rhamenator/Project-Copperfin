#!/usr/bin/env bash
set -u
cd /home/rich/.codex/worktrees/removeitem-numeric-5611/Project-Copperfin
evidence=build/removeitem-native
mkdir -p "$evidence/tmp"
chmod 700 "$evidence/tmp"
export TMPDIR=/home/rich/.codex/worktrees/removeitem-numeric-5611/Project-Copperfin/build/removeitem-native/tmp
export ASAN_OPTIONS=detect_leaks=0
export UBSAN_OPTIONS=print_stacktrace=1
targets=(test_prg_engine_removeitem_numeric test_prg_engine_topitemid_numeric test_prg_engine_topindex_numeric test_prg_engine_listitemid_numeric test_prg_engine_listindex_numeric test_prg_engine_list_index_numeric test_prg_engine_listitem_numeric test_prg_engine_itemidtoindex_numeric test_prg_engine_indextoitemid_numeric test_prg_engine_selectedid_numeric test_prg_engine_selected_index_numeric test_prg_engine_itemdata_index_numeric test_prg_engine_getcolor_default_numeric test_prg_engine_numeric_behavior test_prg_engine_int64_exact_expression test_prg_engine_runtime_surface_functions test_prg_engine_runtime_surface_itemid_selectors)
regex='^(test_native_test_isolation_contract|test_prg_engine_((removeitem|topitemid|topindex|listitemid|listindex|list_index|listitem|itemidtoindex|indextoitemid|selectedid|selected_index|itemdata_index|getcolor_default)_numeric(_.*)?|numeric_behavior|int64_exact_expression|runtime_surface_functions|runtime_surface_itemid_selectors))$'
run_compiler() {
  local name="$1" compiler="$2" flags="$3"
  local build="build/removeitem-$name"
  sha256sum -c "$evidence/review2-fixed-inputs.sha256" > "$evidence/review2-fixed-$name-inputs-before.log" || return 2
  nice -n 10 ionice -c 2 -n 7 cmake -S . -B "$build" -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER="$compiler" -DCMAKE_CXX_FLAGS_DEBUG="$flags" -DCOPPERFIN_BUILD_TESTS=ON > "$evidence/review2-configure-$name.log" 2>&1
  local configure_status=$?
  printf '%s\n' "$configure_status" > "$evidence/review2-configure-$name.status"
  test "$configure_status" -eq 0 || return 3
  nice -n 10 ionice -c 2 -n 7 cmake --build "$build" --parallel 1 --target "${targets[@]}" > "$evidence/review2-compile-$name.log" 2>&1
  local compile_status=$?
  printf '%s\n' "$compile_status" > "$evidence/review2-compile-$name.status"
  test "$compile_status" -eq 0 || return 4
  ctest --test-dir "$build" -N -R "$regex" > "$evidence/review2-fixed-$name-test-list.log" 2>&1
  grep -q 'Total Tests: 70$' "$evidence/review2-fixed-$name-test-list.log" || return 5
  nice -n 10 ionice -c 2 -n 7 ctest --test-dir "$build" -V -R "$regex" --parallel 1 --timeout 700 > "$evidence/review2-fixed-$name.log" 2>&1
  local test_status=$?
  printf '%s\n' "$test_status" > "$evidence/review2-fixed-$name.status"
  date -u +%Y-%m-%dT%H:%M:%SZ > "$evidence/review2-fixed-$name.completed-at"
  sha256sum "$evidence/review2-fixed-$name.log" > "$evidence/review2-fixed-$name.log.sha256"
  sha256sum -c "$evidence/review2-fixed-inputs.sha256" > "$evidence/review2-fixed-$name-inputs-after.log" || return 6
  printf '%s fixed complete with actual ctest status %s\n' "$name" "$test_status"
  # Stop on an actionable fixed-suite failure before proceeding to Clang.
  return "$test_status"
}
run_compiler gcc /usr/bin/c++ '-O1 -g0' || exit "$?"
run_compiler clang /usr/bin/clang++ '-O1 -g0 -fsanitize=address,undefined,float-cast-overflow -fno-omit-frame-pointer' || exit "$?"
printf 'Both identical fixed suites complete; preservation/contracts/integration still required.\n'
