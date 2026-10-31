#!/usr/bin/env bash
# RQ-CF-PRG-LISTITEM-COLUMN-NUMERIC-001 exact-head broader qualification.
# This gate supersedes only the two comment-only fixed544 manifest differences;
# it does not rewrite or retroactively relabel the historical fixed106 evidence.
set -u
root=/home/rich/.codex/worktrees/listitem-column-read-5611/Project-Copperfin
cd "$root" || exit 2
fixture=tests/fixtures/vfp9-listitem-column-numeric
evidence=build/listitem-column-native
broader="$evidence/broader"
mkdir -p "$broader/tmp" || exit 2
chmod 700 "$broader/tmp" || exit 2
[ -z "$(git status --porcelain --untracked-files=no)" ] || exit 3
head=$(git rev-parse HEAD) || exit 3
remote=$(git rev-parse origin/fix/listitem-column-read-5611) || exit 3
[ "$head" = "$remote" ] || exit 3
for gate in \
  fixed/fixed-compile-gcc fixed/fixed-gcc \
  fixed/fixed-compile-clang fixed/fixed-clang fixed/complete-audit; do
  [ "$(< "$evidence/$gate.status")" = 0 ] || exit 4
done
for compiler in gcc clang; do
  for mode in COPPERFIN VFP9; do
    for session in 1 2; do
      [ "$(< "$evidence/hostile-$compiler-$mode-$session.status")" = 0 ] || exit 4
    done
  done
  [ "$(< "$evidence/preservation/preservation-fixed-$compiler.cmp.status")" = 0 ] || exit 4
done
[ "$(< "$evidence/contracts/development-contracts.status")" = 0 ] || exit 4
sha256sum -c "$fixture/broader-inputs.sha256" > "$broader/inputs-preflight.log" || exit 5
targets=(
  test_native_test_isolation_contract
  test_prg_numeric_metadata
  test_prg_numeric_literal_metadata_helper test_prg_engine_numeric_literal_metadata
  test_prg_numeric_unary_metadata_helper test_prg_engine_numeric_unary_metadata
  test_prg_engine_numeric_zero_context test_prg_numeric_expression_context_helper
  test_prg_engine_numeric_integer_metadata test_prg_numeric_integer_metadata_helper
  test_prg_engine_numeric_integer_assignment test_prg_numeric_integer_assignment_helper
  test_prg_engine_numeric_real_assignment test_prg_numeric_real_assignment_helper
  test_prg_engine_numeric_runtime_division test_prg_numeric_runtime_division_helper
  test_prg_engine_numeric_constant_division test_prg_engine_numeric_nonwhole_division
  test_prg_numeric_constant_division_intrinsic_helper
  test_prg_numeric_constant_arithmetic_metadata_helper
  test_prg_numeric_nonwhole_division_helper
  test_prg_engine_numeric_small_divisor test_prg_numeric_small_divisor_helper
  test_prg_numeric_value_display_helper
  test_prg_engine_numeric_literal_payload test_prg_numeric_literal_payload_helper
  test_prg_engine_listitem_column_numeric test_prg_engine_listitem_column_numeric_public
  test_prg_engine_addlistitem_column_numeric test_prg_engine_addlistitem_new_column_numeric
  test_prg_engine_additem_column_numeric test_prg_engine_additem_numeric
  test_prg_engine_addlistitem_numeric test_prg_engine_removelistitem_numeric
  test_prg_engine_removeitem_numeric test_prg_engine_topitemid_numeric
  test_prg_engine_topindex_numeric test_prg_engine_listitemid_numeric
  test_prg_engine_listindex_numeric test_prg_engine_list_index_numeric
  test_prg_engine_listitem_numeric test_prg_engine_itemidtoindex_numeric
  test_prg_engine_indextoitemid_numeric test_prg_engine_selectedid_numeric
  test_prg_engine_selected_index_numeric test_prg_engine_itemdata_index_numeric
  test_prg_engine_getcolor_default_numeric test_prg_engine_numeric_behavior
  test_prg_engine_int64_exact_expression test_prg_engine_runtime_surface_functions
  test_prg_engine_runtime_surface_itemid_selectors
)
regex='^(test_native_test_isolation_contract|test_prg_numeric_metadata|test_prg_numeric_literal_metadata_helper|test_prg_engine_numeric_literal_metadata|test_prg_numeric_unary_metadata_helper|test_prg_engine_numeric_unary_metadata|test_prg_engine_numeric_zero_context|test_prg_numeric_expression_context_helper|test_prg_engine_numeric_integer_metadata|test_prg_numeric_integer_metadata_helper|test_prg_engine_numeric_integer_assignment|test_prg_numeric_integer_assignment_helper|test_prg_engine_numeric_real_assignment|test_prg_numeric_real_assignment_helper|test_prg_engine_numeric_runtime_division|test_prg_numeric_runtime_division_helper|test_prg_engine_numeric_constant_division|test_prg_engine_numeric_nonwhole_division|test_prg_numeric_constant_division_intrinsic_helper|test_prg_numeric_constant_arithmetic_metadata_helper|test_prg_numeric_nonwhole_division_helper|test_prg_engine_numeric_small_divisor|test_prg_numeric_small_divisor_helper|test_prg_numeric_value_display_helper|test_prg_engine_numeric_literal_payload|test_prg_numeric_literal_payload_helper|test_prg_engine_((listitem_column|addlistitem_column|addlistitem_new_column|additem_column|additem|addlistitem|removelistitem|removeitem|topitemid|topindex|listitemid|listindex|list_index|listitem|itemidtoindex|indextoitemid|selectedid|selected_index|itemdata_index|getcolor_default)_numeric(_.*)?|numeric_behavior|int64_exact_expression|runtime_surface_functions|runtime_surface_itemid_selectors))$'
export TMPDIR="$root/$broader/tmp"
export ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=print_stacktrace=1
run_compiler() {
  local name="$1" build="build/listitem-column-$1" result
  sha256sum -c "$fixture/broader-inputs.sha256" > "$broader/$name-inputs-before.log" || return 6
  nice -n 10 ionice -c 2 -n 7 cmake --build "$build" --parallel 1 --target "${targets[@]}" > "$broader/$name-build.log" 2>&1
  result=$?; printf '%s\n' "$result" > "$broader/$name-build.status"; [ "$result" = 0 ] || return 7
  ctest --test-dir "$build" -N -R "$regex" > "$broader/$name-test-list.log" 2>&1 || return 8
  rg -q 'Total Tests: 131$' "$broader/$name-test-list.log" || return 8
  ctest --test-dir "$build" --show-only=json-v1 -R "$regex" > "$broader/$name-test-metadata.json" 2> "$broader/$name-test-metadata.stderr" || return 8
  node "$fixture/audit-test-metadata.cjs" "$broader/$name-test-metadata.json" > "$broader/$name-metadata-audit.jsonl" 2> "$broader/$name-metadata-audit.stderr" || return 8
  nice -n 10 ionice -c 2 -n 7 ctest --test-dir "$build" -V -R "$regex" --parallel 1 --timeout 700 --output-junit "$broader/$name-junit.xml" > "$broader/$name.log" 2>&1
  result=$?; printf '%s\n' "$result" > "$broader/$name.status"
  date -u +%Y-%m-%dT%H:%M:%SZ > "$broader/$name.completed-at"
  sha256sum "$broader/$name.log" "$broader/$name-junit.xml" > "$broader/$name.outputs.sha256"
  printf '%s\n' "$head" > "$broader/$name-product-head"
  sha256sum -c "$fixture/broader-inputs.sha256" > "$broader/$name-inputs-after.log" || return 9
  return "$result"
}
run_compiler gcc; gcc_status=$?
run_compiler clang; clang_status=$?
printf '%s\n' "$head" > "$broader/product-head"
printf '%s\n' "$remote" > "$broader/remote-head"
[ "$gcc_status" = 0 ] && [ "$clang_status" = 0 ] || exit 10
