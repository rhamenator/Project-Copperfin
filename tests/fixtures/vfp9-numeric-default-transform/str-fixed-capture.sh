#!/bin/bash
# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only
# Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
# NEW BOTH native-only CLEAR required; no compiler or completed native replay.
set -eu
test "${1-}" = --fresh-both-clear || exit 93
cd /home/rich/.codex/worktrees/numeric-literal-display-5611/Project-Copperfin
capture_root=build/default-transform-str-fixed
fixture=tests/fixtures/vfp9-numeric-default-transform
observer=build/default-transform-gcvt-captures1/gcvt-probe.fll
mkdir -p "$capture_root"
test ! -e "$capture_root/started.utc" || exit 96
git status --porcelain > "$capture_root/source-status.txt"
test ! -s "$capture_root/source-status.txt" || exit 97
git rev-parse HEAD > "$capture_root/source-commit.txt"
date -u +%FT%TZ > "$capture_root/started.utc"
printf '%s\n' "${WINEDLLOVERRIDES-UNSET}" > "$capture_root/inherited-dll-overrides.txt"
test -z "${WINEDLLOVERRIDES-}" || exit 94
printf 'WINEDLLOVERRIDES=msvcr71=n\n' > "$capture_root/supplied-runtime-env.txt"
printf '%s  %s\n' 0b1fcd2ba81b95631cb52e6ecd0d4afa721b450a967402f504d1852a49e9e9ac "$observer" | sha256sum -c > "$capture_root/reused-binary-check.txt"
for name in gcvt-probe.c gcvt-boundary.h crt-probe.c crt-binding.h; do
 gzip -cd "$fixture/gcvt-captures1/source-$name.gz" > "$capture_root/reused-$name"
 cmp "$fixture/$name" "$capture_root/reused-$name"
done
for name in emit-str-fixed.cjs str-fixed.prg str-fixed-cases.json audit-str-fixed.cjs str-fixed-capture.sh gcvt-cases.json; do
 cp "$fixture/$name" "$capture_root/source-$name"
done
nice -n 10 node "$fixture/emit-str-fixed.cjs" source > "$capture_root/regenerated.prg"
nice -n 10 node "$fixture/emit-str-fixed.cjs" cases > "$capture_root/regenerated-cases.json"
cmp "$fixture/str-fixed.prg" "$capture_root/regenerated.prg"
cmp "$fixture/str-fixed-cases.json" "$capture_root/regenerated-cases.json"
sha256sum "$fixture"/{emit-str-fixed.cjs,str-fixed.prg,str-fixed-cases.json,audit-str-fixed.cjs,str-fixed-capture.sh,gcvt-probe.c,gcvt-boundary.h,crt-probe.c,crt-binding.h,gcvt-cases.json,gcvt-captures1/audit.json.gz,gcvt-captures1/first-results.txt.gz,gcvt-captures1/repeat-results.txt.gz,gcvt-captures1/source-gcvt-probe.c.gz,gcvt-captures1/source-gcvt-boundary.h.gz,gcvt-captures1/source-crt-probe.c.gz,gcvt-captures1/source-crt-binding.h.gz} "$observer" /home/rich/bin/vfp9-probe /home/rich/copperfin-wine-vfp9/drive_c/hotfix968409/vfp9.exe /home/rich/copperfin-wine-vfp9/drive_c/windows/syswow64/msvcr71.dll /home/rich/dev/Project-Copperfin/build/pr7203-completed/native-evidence/diagnostic/cause-probe.fll > "$capture_root/inputs.sha256"
cp "$fixture/str-fixed.prg" "$capture_root/probe.prg"
cmp "$fixture/str-fixed.prg" "$capture_root/probe.prg"
for name in first repeat; do
 date -u +%FT%TZ > "$capture_root/$name.started.utc"
 set +e
 pgrep -a -x 'wine|wine64|wine-preloader|wineserver|wineserver64|vfp9.exe' > "$capture_root/$name.prefix.txt"
 prefix_status=$?
 set -e
 printf '%s\n' "$prefix_status" > "$capture_root/$name.prefix.status"
 test "$prefix_status" -eq 1 || exit 76
 cat /proc/pressure/io > "$capture_root/$name.pressure-before.txt"
 set +e
 awk '$1=="full" {split($2,a,"=");split($3,b,"=");found=1;if(a[2]>=1||b[2]>=1)exit 75} END {if(!found)exit 75}' "$capture_root/$name.pressure-before.txt"
 guard_status=$?
 set -e
 printf '%s\n' "$guard_status" > "$capture_root/$name.guard.status"
 test "$guard_status" -eq 0 || exit 75
 set +e
 nice -n 10 /usr/bin/env WINEDLLOVERRIDES=msvcr71=n /home/rich/bin/vfp9-probe "$capture_root/probe.prg" --result "$capture_root/results.txt" > "$capture_root/$name.out" 2> "$capture_root/$name.stderr"
 status=$?
 set -e
 printf '%s\n' "$status" > "$capture_root/$name.status"
 if [ -f "$capture_root/results.txt" ]; then cp "$capture_root/results.txt" "$capture_root/$name-results.txt"; fi
 cat /proc/pressure/io > "$capture_root/$name.pressure-after.txt"
 date -u +%FT%TZ > "$capture_root/$name.finished.utc"
 printf '%s native status=%s\n' "$name" "$status"
 test "$status" -eq 0 || exit "$status"
 sha256sum -c "$capture_root/inputs.sha256" > "$capture_root/$name.inputs-check.txt"
 if [ "$name" = first ]; then
  set +e
  nice -n 10 node "$fixture/audit-str-fixed.cjs" "$capture_root" --single > "$capture_root/first-structure.json" 2> "$capture_root/first-structure.stderr"
  status=$?
  set -e
  printf '%s\n' "$status" > "$capture_root/first-structure.status"
  test "$status" -eq 0 || exit "$status"
 fi
done
set +e
nice -n 10 node "$fixture/audit-str-fixed.cjs" "$capture_root" > "$capture_root/audit.json" 2> "$capture_root/audit.stderr"
status=$?
set -e
printf '%s\n' "$status" > "$capture_root/audit.status"
date -u +%FT%TZ > "$capture_root/finished.utc"
exit "$status"
