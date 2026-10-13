#!/usr/bin/env bash
set -u
cd /home/rich/.codex/worktrees/additem-numeric-5611/Project-Copperfin || exit 2
phase="$1"
name="$2"
[ "$phase" = baseline ] || [ "$phase" = fixed ] || exit 3
compiler=/usr/bin/c++
flags=(-std=c++20 -O1 -g0)
if [ "$name" = clang ]; then
 compiler=/usr/bin/clang++
 flags+=(-fsanitize=address,undefined,float-cast-overflow -fno-omit-frame-pointer)
elif [ "$name" != gcc ]; then
 exit 4
fi
evidence=build/additem-native
mkdir -p "$evidence/preservation-supplemental-$phase-$name-tmp"
chmod 700 "$evidence/preservation-supplemental-$phase-$name-tmp"
export TMPDIR=/home/rich/.codex/worktrees/additem-numeric-5611/Project-Copperfin/$evidence/preservation-supplemental-$phase-$name-tmp
export ASAN_OPTIONS=detect_leaks=0
export UBSAN_OPTIONS=print_stacktrace=1
mapfile -t libraries < <(rg --files "build/additem-$name" -g '*.a' | sort)
[ "${#libraries[@]}" = 9 ] || exit 5
sha256sum -c "$evidence/preservation-supplemental-source-frozen.sha256" > "$evidence/preservation-supplemental-$phase-$name-source-before.log" || exit 6
sha256sum "${libraries[@]}" > "$evidence/preservation-supplemental-$phase-$name-libraries.sha256"
nice -n 10 ionice -c 2 -n 7 "$compiler" "${flags[@]}" -Iinclude -Itests -Isrc/runtime "$evidence/excluded-scope-preservation-supplemental.cpp" tests/prg_engine_test_support.cpp -Wl,--start-group "${libraries[@]}" -Wl,--end-group -pthread -ldl -lz -o "$evidence/preservation-supplemental-$phase-$name" > "$evidence/preservation-supplemental-$phase-$name-compile.log" 2>&1
status=$?
printf '%s\n' "$status" > "$evidence/preservation-supplemental-$phase-$name-compile.status"
[ "$status" = 0 ] || exit 7
nice -n 10 ionice -c 2 -n 7 "$evidence/preservation-supplemental-$phase-$name" "/home/rich/.codex/worktrees/additem-numeric-5611/Project-Copperfin/$evidence/preservation-supplemental-$phase-$name-scripts" > "$evidence/preservation-supplemental-$phase-$name.out" 2> "$evidence/preservation-supplemental-$phase-$name.stderr"
status=$?
printf '%s\n' "$status" > "$evidence/preservation-supplemental-$phase-$name.status"
sha256sum "$evidence/preservation-supplemental-$phase-$name.out" > "$evidence/preservation-supplemental-$phase-$name.out.sha256"
sha256sum -c "$evidence/preservation-supplemental-$phase-$name-libraries.sha256" > "$evidence/preservation-supplemental-$phase-$name-libraries-after.log" || exit 8
sha256sum -c "$evidence/preservation-supplemental-source-frozen.sha256" > "$evidence/preservation-supplemental-$phase-$name-source-after.log" || exit 9
date -u +%Y-%m-%dT%H:%M:%SZ > "$evidence/preservation-supplemental-$phase-$name.completed-at"
if [ "$phase" = fixed ]; then
 cmp "$evidence/preservation-supplemental-baseline-$name.out" "$evidence/preservation-supplemental-fixed-$name.out" > "$evidence/preservation-supplemental-fixed-$name.cmp.log" 2>&1
 cmp_status=$?
 printf '%s\n' "$cmp_status" > "$evidence/preservation-supplemental-fixed-$name.cmp.status"
 [ "$cmp_status" = 0 ] || exit 10
fi
exit "$status"
