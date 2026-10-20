#!/bin/bash
# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only
# Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
# RQ-CF-PRG-NUMERIC-DEFAULT-TRANSFORM-001: original payloads DECIMALS12/14 metadata13/14, width40 ONLY.
# NEW BOTH native-only CLEAR plus Request Guard release/coordination required.
# This flag acknowledges external coordination; it never grants that clearance.
set -euo pipefail
test "${1-}" = --fresh-both-clear || exit 93
cd /home/rich/.codex/worktrees/numeric-literal-display-5611/Project-Copperfin
capture_root=build/default-transform-original-setting-consumer
fixture=tests/fixtures/vfp9-numeric-default-transform
observer=build/default-transform-gcvt-captures1/gcvt-probe.fll
test ! -e "$capture_root" || exit 96
mkdir "$capture_root"
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
gzip -cd "$fixture/round-inline-captures1/source-round-inline.prg.gz" > "$capture_root/reused-round-inline.prg"
cmp "$fixture/round-inline.prg" "$capture_root/reused-round-inline.prg"
for name in emit-original-setting-consumer.cjs original-setting-consumer.prg original-setting-consumer-cases.json audit-original-setting-consumer.cjs check-original-setting-reader.cjs check-original-setting-worker.cjs check-original-setting-source.cjs original-setting-consumer-capture.sh original-setting-source-check.sh round-inline.prg str-middle-cases.json; do
 cp "$fixture/$name" "$capture_root/source-$name"
done
sha256sum "$fixture"/{emit-original-setting-consumer.cjs,original-setting-consumer.prg,original-setting-consumer-cases.json,audit-original-setting-consumer.cjs,check-original-setting-reader.cjs,check-original-setting-worker.cjs,check-original-setting-source.cjs,original-setting-consumer-capture.sh,original-setting-source-check.sh,round-inline.prg,str-middle-cases.json} \
 "$fixture"/round-inline-captures1/{SHA256SUMS,audit.json.gz,first-results.txt.gz,repeat-results.txt.gz,first.out.gz,repeat.out.gz,first.status.gz,repeat.status.gz,first.stderr.gz,repeat.stderr.gz,source-round-inline.prg.gz} \
 "$fixture"/gcvt-captures1/source-{gcvt-probe.c,gcvt-boundary.h,crt-probe.c,crt-binding.h}.gz \
 "$observer" /home/rich/bin/vfp9-probe /home/rich/copperfin-wine-vfp9/drive_c/hotfix968409/vfp9.exe /home/rich/copperfin-wine-vfp9/drive_c/windows/syswow64/msvcr71.dll /home/rich/dev/Project-Copperfin/build/pr7203-completed/native-evidence/diagnostic/cause-probe.fll > "$capture_root/inputs.sha256"
checked() {
 local name=$1 output=$2
 shift 2
 sha256sum -c "$capture_root/inputs.sha256" > "$capture_root/$name.inputs-before.txt"
 set +e
 pgrep -a -x 'wine|wine64|wine-preloader|wineserver|wineserver64|vfp9.exe' > "$capture_root/$name.prefix.txt"
 local prefix_status=$?
 set -e
 printf '%s\n' "$prefix_status" > "$capture_root/$name.prefix.status"
 test "$prefix_status" -eq 1 || exit 76
 cat /proc/pressure/io > "$capture_root/$name.pressure-before.txt"
 set +e
 awk '$1=="full" {split($2,a,"=");split($3,b,"=");found=1;if(a[2]>=1||b[2]>=1)exit 75} END {if(!found)exit 75}' "$capture_root/$name.pressure-before.txt"
 local guard_status=$?
 set -e
 printf '%s\n' "$guard_status" > "$capture_root/$name.guard.status"
 test "$guard_status" -eq 0 || exit 75
 set +e
 nice -n 10 "$@" > "$capture_root/$output" 2> "$capture_root/$name.stderr"
 local status=$?
 set -e
 printf '%s\n' "$status" > "$capture_root/$name.status"
 # Keep any partial native report even when the wrapper exits nonzero.
 if [[ "$name" = first || "$name" = repeat ]] && [ -f "$capture_root/results.txt" ]; then cp "$capture_root/results.txt" "$capture_root/$name-results.txt"; fi
 test "$status" -eq 0 || exit "$status"
 test ! -s "$capture_root/$name.stderr" || exit 98
 sha256sum -c "$capture_root/inputs.sha256" > "$capture_root/$name.inputs-after.txt"
}
checked regenerate-source regenerated.prg node "$fixture/emit-original-setting-consumer.cjs" source
checked regenerate-cases regenerated-cases.json node "$fixture/emit-original-setting-consumer.cjs" cases
cmp "$fixture/original-setting-consumer.prg" "$capture_root/regenerated.prg"
cmp "$fixture/original-setting-consumer-cases.json" "$capture_root/regenerated-cases.json"
checked parent-source-check parent-source-check.json node "$fixture/audit-original-setting-consumer.cjs" --source-only
cp "$fixture/original-setting-consumer.prg" "$capture_root/probe.prg"
cmp "$fixture/original-setting-consumer.prg" "$capture_root/probe.prg"
for name in first repeat; do
 date -u +%FT%TZ > "$capture_root/$name.started.utc"
 checked "$name" "$name.out" /usr/bin/env WINEDLLOVERRIDES=msvcr71=n /home/rich/bin/vfp9-probe "$capture_root/probe.prg" --result "$capture_root/results.txt"
 cat /proc/pressure/io > "$capture_root/$name.pressure-after.txt"
 date -u +%FT%TZ > "$capture_root/$name.finished.utc"
 printf '%s native status=0\n' "$name"
 if [ "$name" = first ]; then
  checked first-structure first-structure.json node "$fixture/audit-original-setting-consumer.cjs" "$capture_root" --single
 fi
done
checked audit audit.json node "$fixture/audit-original-setting-consumer.cjs" "$capture_root"
date -u +%FT%TZ > "$capture_root/finished.utc"
