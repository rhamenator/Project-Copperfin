#!/bin/bash
# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only
# Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
# NEW explicit audit+DAW CLEAR required; do not rerun an attempted directory.
set -eu
cd /home/rich/.codex/worktrees/numeric-literal-display-5611/Project-Copperfin
capture_root=build/default-transform-crt-precision
fixture=tests/fixtures/vfp9-numeric-default-transform
test -e "$capture_root/build.finished.utc"
test ! -e "$capture_root/first.started.utc" || exit 96
git status --porcelain > "$capture_root/source-status.txt"
test ! -s "$capture_root/source-status.txt" || exit 97
git rev-parse HEAD > "$capture_root/source-commit.txt"
date -u +%FT%TZ > "$capture_root/started.utc"
printf '%s\n' "${WINEDLLOVERRIDES-UNSET}" > "$capture_root/inherited-dll-overrides.txt"
test -z "${WINEDLLOVERRIDES-}" || exit 94
printf 'WINEDLLOVERRIDES=msvcr71=n\n' > "$capture_root/supplied-runtime-env.txt"
sha256sum -c "$capture_root/build-inputs.sha256" > "$capture_root/pre-native-build-check.txt"
sha256sum -c "$capture_root/built.sha256" > "$capture_root/pre-native-binary-check.txt"
sha256sum "$fixture"/{crt-probe.c,crt-binding.h,crt-binding-check.c,crt-binding-check.cjs,crt-kernel32.def,emit-crt-precision.cjs,crt-precision.prg,crt-precision-cases.json,audit-crt-precision.cjs,crt-precision-build.sh,crt-precision-capture.sh,crt-cases.json,crt.prg} /home/rich/bin/vfp9-probe /home/rich/copperfin-wine-vfp9/drive_c/hotfix968409/vfp9.exe /home/rich/copperfin-wine-vfp9/drive_c/windows/syswow64/msvcr71.dll /home/rich/dev/Project-Copperfin/build/pr7203-completed/native-evidence/diagnostic/cause-probe.fll "$capture_root/crt-probe.fll" > "$capture_root/inputs.sha256"
cp "$fixture/crt-precision.prg" "$capture_root/probe.prg"
cmp "$fixture/crt-precision.prg" "$capture_root/probe.prg"
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
  node "$fixture/audit-crt-precision.cjs" "$capture_root" --single > "$capture_root/first-structure.json" 2> "$capture_root/first-structure.stderr"
  status=$?
  set -e
  printf '%s\n' "$status" > "$capture_root/first-structure.status"
  test "$status" -eq 0 || exit "$status"
 fi
done
set +e
node "$fixture/audit-crt-precision.cjs" "$capture_root" > "$capture_root/audit.json" 2> "$capture_root/audit.stderr"
status=$?
set -e
printf '%s\n' "$status" > "$capture_root/audit.status"
date -u +%FT%TZ > "$capture_root/finished.utc"
exit "$status"
