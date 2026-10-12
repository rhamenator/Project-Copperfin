#!/usr/bin/env bash
set -u
cd /home/rich/.codex/worktrees/removeitem-numeric-5611/Project-Copperfin || exit 2
e=build/removeitem-native
while [ ! -f "$e/review-fixed-driver.status" ]; do sleep 30; done
[ "$(< "$e/review-fixed-driver.status")" = 0 ] || exit 3
export TMPDIR=/home/rich/.codex/worktrees/removeitem-numeric-5611/Project-Copperfin/build/removeitem-native/tmp
export ASAN_OPTIONS=detect_leaks=0
export UBSAN_OPTIONS=print_stacktrace=1
for name in gcc clang; do
  compiler=/usr/bin/c++
  flags=()
  if [ "$name" = clang ]; then compiler=/usr/bin/clang++; flags=(-fsanitize=address,undefined,float-cast-overflow -fno-omit-frame-pointer); fi
  sha256sum -c "$e/review-fixed-inputs.sha256" > "$e/review-preservation-$name-inputs-before.log" || exit 4
  for group in scope supplemental sixgroups; do
    source="$e/excluded-scope-preservation.cpp"
    extra=()
    arguments=()
    if [ "$group" = scope ]; then arguments=("$e/review-scope-scripts-$name"); fi
    if [ "$group" = supplemental ]; then source="$e/review-blank-driver.cpp"; fi
    if [ "$group" = sixgroups ]; then source="$e/preservation-driver.cpp"; extra=(tests/test_prg_engine_runtime_surface_functions_list_controls_03.cpp); fi
    nice -n 10 ionice -c 2 -n 7 "$compiler" -std=c++20 -O1 -g0 "${flags[@]}" -Iinclude -Itests -Isrc/runtime "$source" "${extra[@]}" tests/prg_engine_test_support.cpp -Wl,--start-group build/removeitem-$name/*.a -Wl,--end-group -pthread -ldl -lz -o "$e/review-$group-$name" > "$e/review-$group-$name-compile.log" 2>&1
    result=$?
    printf '%s\n' "$result" > "$e/review-$group-$name-compile.status"
    [ "$result" = 0 ] || exit 5
    nice -n 10 ionice -c 2 -n 7 "$e/review-$group-$name" "${arguments[@]}" > "$e/review-$group-$name.log" 2>&1
    result=$?
    printf '%s\n' "$result" > "$e/review-$group-$name.status"
    date -u +%Y-%m-%dT%H:%M:%SZ > "$e/review-$group-$name.completed-at"
    sha256sum "$e/review-$group-$name.log" > "$e/review-$group-$name.log.sha256"
    [ "$result" = 0 ] || exit 6
    if [ "$group" = scope ]; then
      cmp "$e/scope-baseline.out" "$e/review-$group-$name.log" > "$e/review-$group-$name-cmp.log" 2>&1
      result=$?
      printf '%s\n' "$result" > "$e/review-$group-$name-cmp.status"
      [ "$result" = 0 ] || exit 7
    fi
  done
  sha256sum -c "$e/review-fixed-inputs.sha256" > "$e/review-preservation-$name-inputs-after.log" || exit 8
done
sha256sum -c "$e/review-fixed-inputs.sha256" > "$e/review-contracts-inputs-before.log" || exit 9
nice -n 10 ionice -c 2 -n 7 ctest --test-dir build/removeitem-gcc -V --parallel 1 -R '^(test_contributor_signoff_contract|test_changelog_fragment_assembler|test_changelog_fragments_valid|test_agent_channel_contract|test_safety_traceability_workflow_contract)$' > "$e/review-contracts.log" 2>&1
result=$?
printf '%s\n' "$result" > "$e/review-contracts.status"
date -u +%Y-%m-%dT%H:%M:%SZ > "$e/review-contracts.completed-at"
sha256sum "$e/review-contracts.log" > "$e/review-contracts.log.sha256"
sha256sum -c "$e/review-fixed-inputs.sha256" > "$e/review-contracts-inputs-after.log" || exit 10
exit "$result"
