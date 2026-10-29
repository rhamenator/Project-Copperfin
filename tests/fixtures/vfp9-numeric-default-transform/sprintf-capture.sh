#!/bin/bash
# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only
# RQ-CF-PRG-NUMERIC-DEFAULT-TRANSFORM-001: bounded public CRT capture.
set -euo pipefail
test "${1-}" = --fresh-resource-clear || exit 93
cd /home/rich/.codex/worktrees/numeric-literal-display-5611/Project-Copperfin
capture_root=build/default-transform-sprintf
fixture=tests/fixtures/vfp9-numeric-default-transform
observer="$capture_root/sprintf-probe.fll"
cause_observer=/home/rich/dev/Project-Copperfin/build/pr7203-completed/native-evidence/diagnostic/cause-probe.fll
runtime=/home/rich/copperfin-wine-vfp9/drive_c/windows/syswow64/msvcr71.dll
lock=/home/rich/.codex/implementation-loops/copperfin-wine-vfp9.lock
exec 9>"$lock"
flock -n 9 || exit 74
test -d "$capture_root" || exit 91
test ! -e "$capture_root/started.utc" || exit 96
git status --porcelain > "$capture_root/source-status.txt"
test ! -s "$capture_root/source-status.txt" || exit 97
git rev-parse HEAD > "$capture_root/source-commit.txt"
date -u +%FT%TZ > "$capture_root/started.utc"
printf '%s\n' "${WINEDLLOVERRIDES-UNSET}" > "$capture_root/inherited-dll-overrides.txt"
test -z "${WINEDLLOVERRIDES-}" || exit 94
printf 'WINEDLLOVERRIDES=msvcr71=n\n' > "$capture_root/supplied-runtime-env.txt"
test -f "$capture_root/build.finished.utc"
cmp "$capture_root/build-source-commit.txt" "$capture_root/source-commit.txt"
for stage in host-gnu check-gnu host-clang check-clang boundary-gnu-build boundary-gnu-check boundary-clang-build boundary-clang-check import compile link; do
 test "$(< "$capture_root/$stage.status")" = 0 || exit 92
done
sha256sum -c "$capture_root/build-inputs.sha256" > "$capture_root/pre-native-build-inputs-check.txt"
sha256sum -c "$capture_root/built.sha256" > "$capture_root/pre-native-built-check.txt"
sha256sum "$fixture"/{emit-sprintf.cjs,sprintf.prg,sprintf-cases.json,read-sprintf-record.cjs,audit-sprintf.cjs,sprintf-capture.sh,gcvt-probe.c,sprintf-boundary.h,crt-binding.h,crt-cases.json} \
 "$observer" "$cause_observer" "$runtime" /home/rich/bin/vfp9-probe \
 /home/rich/copperfin-wine-vfp9/drive_c/hotfix968409/vfp9.exe > "$capture_root/inputs.sha256"
cp "$fixture/sprintf.prg" "$capture_root/probe.prg"
cmp "$fixture/sprintf.prg" "$capture_root/probe.prg"

preflight() {
 local name=$1 prefix_status guard_status memory_status
 sha256sum -c "$capture_root/inputs.sha256" > "$capture_root/$name.inputs-before.txt"
 set +e
 pgrep -a -x 'wine|wine64|wine-preloader|wineserver|wineserver64|vfp9.exe' > "$capture_root/$name.prefix-before.txt"
 prefix_status=$?
 set -e
 printf '%s\n' "$prefix_status" > "$capture_root/$name.prefix-before.status"
 test "$prefix_status" -eq 1 || exit 76
 cat /proc/pressure/cpu > "$capture_root/$name.cpu-pressure-before.txt"
 cat /proc/pressure/io > "$capture_root/$name.io-pressure-before.txt"
 cat /proc/pressure/memory > "$capture_root/$name.memory-pressure-before.txt"
 cat /proc/meminfo > "$capture_root/$name.meminfo-before.txt"
 set +e
 awk '$1=="full" {split($2,a,"=");split($3,b,"=");found=1;if(a[2]>=1||b[2]>=1)exit 75} END {if(!found)exit 75}' "$capture_root/$name.io-pressure-before.txt"
 guard_status=$?
 awk '$1=="MemAvailable:" {found=1;if($2<8388608)exit 75} END {if(!found)exit 75}' "$capture_root/$name.meminfo-before.txt"
 memory_status=$?
 set -e
 printf '%s\n' "$guard_status" > "$capture_root/$name.io-guard.status"
 printf '%s\n' "$memory_status" > "$capture_root/$name.memory-guard.status"
 test "$guard_status" -eq 0 || exit 75
 test "$memory_status" -eq 0 || exit 75
}

native_run() {
 local name=$1 status prefix_status
 date -u +%FT%TZ > "$capture_root/$name.started.utc"
 preflight "$name"
 set +e
 nice -n 10 /usr/bin/env WINEDLLOVERRIDES=msvcr71=n /home/rich/bin/vfp9-probe "$capture_root/probe.prg" --result "$capture_root/results.txt" > "$capture_root/$name.out" 2> "$capture_root/$name.stderr"
 status=$?
 set -e
 printf '%s\n' "$status" > "$capture_root/$name.status"
 if [ -f "$capture_root/results.txt" ]; then cp "$capture_root/results.txt" "$capture_root/$name-results.txt"; fi
 cat /proc/pressure/io > "$capture_root/$name.io-pressure-after.txt"
 set +e
 pgrep -a -x 'wine|wine64|wine-preloader|wineserver|wineserver64|vfp9.exe' > "$capture_root/$name.prefix-after.txt"
 prefix_status=$?
 set -e
 printf '%s\n' "$prefix_status" > "$capture_root/$name.prefix-after.status"
 date -u +%FT%TZ > "$capture_root/$name.finished.utc"
 test "$status" -eq 0 || exit "$status"
 test ! -s "$capture_root/$name.stderr" || exit 98
 test "$prefix_status" -eq 1 || exit 76
 sha256sum -c "$capture_root/inputs.sha256" > "$capture_root/$name.inputs-after.txt"
}

native_run first
set +e
nice -n 10 node "$fixture/audit-sprintf.cjs" "$capture_root" --single > "$capture_root/first-structure.json" 2> "$capture_root/first-structure.stderr"
status=$?
set -e
printf '%s\n' "$status" > "$capture_root/first-structure.status"
test "$status" -eq 0 || exit "$status"
test ! -s "$capture_root/first-structure.stderr" || exit 98

# The repeat is unreachable unless the exact first-run mismatch receipt passes.
native_run repeat
set +e
nice -n 10 node "$fixture/audit-sprintf.cjs" "$capture_root" > "$capture_root/audit.json" 2> "$capture_root/audit.stderr"
status=$?
set -e
printf '%s\n' "$status" > "$capture_root/audit.status"
date -u +%FT%TZ > "$capture_root/finished.utc"
test ! -s "$capture_root/audit.stderr" || exit 98
exit "$status"
