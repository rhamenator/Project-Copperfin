#!/bin/bash
# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only
# Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
# RQ-CF-PRG-NUMERIC-DEFAULT-TRANSFORM-001: bounded ambient STR capture with a mandatory first-run gate.
set -euo pipefail
test "${1-}" = --fresh-both-clear || exit 93
cd /home/rich/.codex/worktrees/numeric-literal-display-5611/Project-Copperfin
capture_root=build/default-transform-str-ambient
fixture=tests/fixtures/vfp9-numeric-default-transform
source_archive="$fixture/str-ambient-source-captures1/generated-source.out.gz"
gcvt_observer=build/default-transform-gcvt-captures1/gcvt-probe.fll
cause_observer=/home/rich/dev/Project-Copperfin/build/pr7203-completed/native-evidence/diagnostic/cause-probe.fll
runtime=/home/rich/copperfin-wine-vfp9/drive_c/windows/syswow64/msvcr71.dll
test ! -e "$capture_root" || exit 96
mkdir "$capture_root"
git status --porcelain > "$capture_root/source-status.txt"
test ! -s "$capture_root/source-status.txt" || exit 97
git rev-parse HEAD > "$capture_root/source-commit.txt"
date -u +%FT%TZ > "$capture_root/started.utc"
printf '%s\n' "${WINEDLLOVERRIDES-UNSET}" > "$capture_root/inherited-dll-overrides.txt"
test -z "${WINEDLLOVERRIDES-}" || exit 94
printf 'WINEDLLOVERRIDES=msvcr71=n\n' > "$capture_root/supplied-runtime-env.txt"

for spec in \
 '0b1fcd2ba81b95631cb52e6ecd0d4afa721b450a967402f504d1852a49e9e9ac build/default-transform-gcvt-captures1/gcvt-probe.fll' \
 '1afd8eca71d2ef716efc22bb90f7f092322d9639ebec671fb5bd1bf4df806c19 /home/rich/dev/Project-Copperfin/build/pr7203-completed/native-evidence/diagnostic/cause-probe.fll' \
 '8094af5ee310714caebccaeee7769ffb08048503ba478b879edfef5f1a24fefe /home/rich/copperfin-wine-vfp9/drive_c/windows/syswow64/msvcr71.dll'; do
 printf '%s\n' "$spec" | sha256sum -c
done > "$capture_root/reused-binary-check.txt"

for name in emit-str-ambient.cjs select-str-ambient-witnesses.cjs read-str-ambient-record.cjs read-str-ambient-run.cjs audit-str-ambient.cjs str-ambient-capture.sh str-middle.prg; do
 cp "$fixture/$name" "$capture_root/source-$name"
done
gzip -cd "$source_archive" > "$capture_root/archived-source.prg"
node "$fixture/emit-str-ambient.cjs" source > "$capture_root/supplied-source.prg"
cmp "$capture_root/archived-source.prg" "$capture_root/supplied-source.prg"
cp "$capture_root/supplied-source.prg" "$capture_root/probe.prg"
cmp "$capture_root/supplied-source.prg" "$capture_root/probe.prg"

sha256sum \
 "$fixture"/{emit-str-ambient.cjs,select-str-ambient-witnesses.cjs,read-str-ambient-record.cjs,read-str-ambient-run.cjs,audit-str-ambient.cjs,str-ambient-capture.sh,str-middle.prg} \
 "$fixture"/str-ambient-selection-captures1/{manifest.json,selection.out.gz} \
 "$fixture"/str-ambient-source-captures1/{manifest.json,generated-source.out.gz} \
 "$gcvt_observer" "$cause_observer" "$runtime" \
 /home/rich/bin/vfp9-probe /home/rich/copperfin-wine-vfp9/drive_c/hotfix968409/vfp9.exe > "$capture_root/inputs.sha256"

preflight() {
 local name=$1
 local prefix_status guard_status
 sha256sum -c "$capture_root/inputs.sha256" > "$capture_root/$name.inputs-before.txt"
 set +e
 pgrep -a -x 'wine|wine64|wine-preloader|wineserver|wineserver64|vfp9.exe' > "$capture_root/$name.prefix.txt"
 prefix_status=$?
 set -e
 printf '%s\n' "$prefix_status" > "$capture_root/$name.prefix.status"
 test "$prefix_status" -eq 1 || exit 76
 cat /proc/pressure/cpu > "$capture_root/$name.cpu-pressure-before.txt"
 cat /proc/pressure/io > "$capture_root/$name.io-pressure-before.txt"
 cat /proc/pressure/memory > "$capture_root/$name.memory-pressure-before.txt"
 free -b > "$capture_root/$name.memory-before.txt"
 set +e
 awk '$1=="full" {split($2,a,"=");split($3,b,"=");found=1;if(a[2]>=1||b[2]>=1)exit 75} END {if(!found)exit 75}' "$capture_root/$name.io-pressure-before.txt"
 guard_status=$?
 set -e
 printf '%s\n' "$guard_status" > "$capture_root/$name.guard.status"
 test "$guard_status" -eq 0 || exit 75
}

native_run() {
 local name=$1
 date -u +%FT%TZ > "$capture_root/$name.started.utc"
 preflight "$name"
 set +e
 nice -n 10 /usr/bin/env WINEDLLOVERRIDES=msvcr71=n /home/rich/bin/vfp9-probe "$capture_root/probe.prg" --result "$capture_root/results.txt" > "$capture_root/$name.out" 2> "$capture_root/$name.stderr"
 local status=$?
 set -e
 printf '%s\n' "$status" > "$capture_root/$name.status"
 if [ -f "$capture_root/results.txt" ]; then cp "$capture_root/results.txt" "$capture_root/$name-results.txt"; fi
 cat /proc/pressure/io > "$capture_root/$name.io-pressure-after.txt"
 date -u +%FT%TZ > "$capture_root/$name.finished.utc"
 test "$status" -eq 0 || exit "$status"
 test ! -s "$capture_root/$name.stderr" || exit 98
 sha256sum -c "$capture_root/inputs.sha256" > "$capture_root/$name.inputs-after.txt"
}

native_run first
set +e
nice -n 10 node "$fixture/audit-str-ambient.cjs" "$capture_root" --single > "$capture_root/first-structure.json" 2> "$capture_root/first-structure.stderr"
status=$?
set -e
printf '%s\n' "$status" > "$capture_root/first-structure.status"
test "$status" -eq 0 || exit "$status"
test ! -s "$capture_root/first-structure.stderr" || exit 98

# The repeat is intentionally unreachable unless the exact first-run reader passes.
native_run repeat
set +e
nice -n 10 node "$fixture/audit-str-ambient.cjs" "$capture_root" > "$capture_root/audit.json" 2> "$capture_root/audit.stderr"
status=$?
set -e
printf '%s\n' "$status" > "$capture_root/audit.status"
date -u +%FT%TZ > "$capture_root/finished.utc"
test ! -s "$capture_root/audit.stderr" || exit 98
exit "$status"
