#!/usr/bin/env bash
set -u
cd /home/rich/.codex/worktrees/removelistitem-numeric-5611/Project-Copperfin || exit 2
evidence=build/removelistitem-native
name="$1"
compiler=/usr/bin/c++
flags=(-std=c++20 -O1 -g0)
if [ "$name" = clang ]; then
  compiler=/usr/bin/clang++
  flags+=(-fsanitize=address,undefined,float-cast-overflow -fno-omit-frame-pointer)
elif [ "$name" != gcc ]; then
  exit 3
fi
export TMPDIR=/home/rich/.codex/worktrees/removelistitem-numeric-5611/Project-Copperfin/build/removelistitem-native/tmp
export ASAN_OPTIONS=detect_leaks=0
export UBSAN_OPTIONS=print_stacktrace=1
mapfile -t libraries < <(rg --files "build/removelistitem-$name" -g '*.a' | sort)
[ "${#libraries[@]}" = 9 ] || exit 4
sha256sum "${libraries[@]}" > "$evidence/preservation-$name-libraries.sha256"
sha256sum -c "$evidence/refrozen-fixed-inputs.sha256" > "$evidence/preservation-$name-inputs-before.log" || exit 5
for kind in scope groups; do
  sources=("$evidence/excluded-scope-preservation.cpp")
  if [ "$kind" = groups ]; then
    sources=("$evidence/preservation-driver.cpp" tests/test_prg_engine_runtime_surface_functions_list_controls_03.cpp)
  fi
  nice -n 10 ionice -c 2 -n 7 "$compiler" "${flags[@]}" -Iinclude -Itests -Isrc/runtime "${sources[@]}" tests/prg_engine_test_support.cpp -Wl,--start-group "${libraries[@]}" -Wl,--end-group -pthread -ldl -lz -o "$evidence/preservation-$name-$kind" > "$evidence/preservation-$name-$kind-compile.log" 2>&1
  status=$?
  printf '%s\n' "$status" > "$evidence/preservation-$name-$kind-compile.status"
  [ "$status" = 0 ] || exit 6
  args=()
  if [ "$kind" = scope ]; then
    args=("/home/rich/.codex/worktrees/removelistitem-numeric-5611/Project-Copperfin/build/removelistitem-native/preservation-$name-scripts")
  fi
  nice -n 10 ionice -c 2 -n 7 "$evidence/preservation-$name-$kind" "${args[@]}" > "$evidence/preservation-$name-$kind.out" 2> "$evidence/preservation-$name-$kind.stderr"
  status=$?
  printf '%s\n' "$status" > "$evidence/preservation-$name-$kind.status"
  [ "$status" = 0 ] || exit 7
  sha256sum "$evidence/preservation-$name-$kind.out" > "$evidence/preservation-$name-$kind.out.sha256"
done
cmp "$evidence/scope-baseline.out" "$evidence/preservation-$name-scope.out"
status=$?
printf '%s\n' "$status" > "$evidence/preservation-$name-scope.cmp.status"
[ "$status" = 0 ] || exit 8
sha256sum -c "$evidence/preservation-$name-libraries.sha256" > "$evidence/preservation-$name-libraries-after.log" || exit 9
sha256sum -c "$evidence/refrozen-fixed-inputs.sha256" > "$evidence/preservation-$name-inputs-after.log" || exit 10
date -u +%Y-%m-%dT%H:%M:%SZ > "$evidence/preservation-$name.completed-at"
