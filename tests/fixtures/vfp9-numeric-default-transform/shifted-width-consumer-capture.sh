#!/bin/bash
# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only
# Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
# NEW BOTH native-only CLEAR required; no compiler or completed native replay.
set -euo pipefail
test "${1-}" = --fresh-both-clear || exit 93
cd /home/rich/.codex/worktrees/numeric-literal-display-5611/Project-Copperfin
capture_root=build/default-transform-shifted-width-consumer
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
for name in emit-shifted-width-consumer.cjs shifted-width-consumer.prg shifted-width-consumer-cases.json audit-shifted-width-consumer.cjs check-shifted-width-reader.cjs shifted-width-consumer-capture.sh round-inline.prg str-middle-cases.json; do
 cp "$fixture/$name" "$capture_root/source-$name"
done
nice -n 10 node "$fixture/emit-shifted-width-consumer.cjs" source > "$capture_root/regenerated.prg"
nice -n 10 node "$fixture/emit-shifted-width-consumer.cjs" cases > "$capture_root/regenerated-cases.json"
cmp "$fixture/shifted-width-consumer.prg" "$capture_root/regenerated.prg"
cmp "$fixture/shifted-width-consumer-cases.json" "$capture_root/regenerated-cases.json"
nice -n 10 node "$fixture/audit-shifted-width-consumer.cjs" --source-only > "$capture_root/parent-source-check.json"
# Retained stage validator inventory covers all nested source/raw dependencies.
gzip -cd "$fixture/stage-adjacency-analysis1/inputs.sha256.gz" > "$capture_root/prior-inputs.sha256"
sha256sum -c "$capture_root/prior-inputs.sha256" > "$capture_root/prior-inputs-check.txt"
(
 cat "$capture_root/prior-inputs.sha256"
 sha256sum "$fixture"/{emit-shifted-width-consumer.cjs,shifted-width-consumer.prg,shifted-width-consumer-cases.json,audit-shifted-width-consumer.cjs,check-shifted-width-reader.cjs,shifted-width-consumer-capture.sh,round-inline.prg,str-middle-cases.json,stage-adjacency-analysis1/manifest.json,stage-adjacency-analysis1/source-join-observable-stage-adjacency.cjs.gz,stage-adjacency-analysis1/analysis.json.gz,arithmetic-inline-captures1/SHA256SUMS,arithmetic-inline-captures1/audit.json.gz,arithmetic-inline-captures1/first-results.txt.gz,arithmetic-inline-captures1/repeat-results.txt.gz,arithmetic-inline-captures1/first.out.gz,arithmetic-inline-captures1/repeat.out.gz,arithmetic-inline-captures1/first.status.gz,arithmetic-inline-captures1/repeat.status.gz,arithmetic-inline-captures1/first.stderr.gz,arithmetic-inline-captures1/repeat.stderr.gz,gcvt-captures1/source-gcvt-probe.c.gz,gcvt-captures1/source-gcvt-boundary.h.gz,gcvt-captures1/source-crt-probe.c.gz,gcvt-captures1/source-crt-binding.h.gz} "$observer" /home/rich/bin/vfp9-probe /home/rich/copperfin-wine-vfp9/drive_c/hotfix968409/vfp9.exe /home/rich/copperfin-wine-vfp9/drive_c/windows/syswow64/msvcr71.dll /home/rich/dev/Project-Copperfin/build/pr7203-completed/native-evidence/diagnostic/cause-probe.fll
) | sort -u > "$capture_root/inputs.sha256"
sha256sum -c "$capture_root/inputs.sha256" > "$capture_root/inputs-check.txt"
cp "$fixture/shifted-width-consumer.prg" "$capture_root/probe.prg"
cmp "$fixture/shifted-width-consumer.prg" "$capture_root/probe.prg"
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
  nice -n 10 node "$fixture/audit-shifted-width-consumer.cjs" "$capture_root" --single > "$capture_root/first-structure.json" 2> "$capture_root/first-structure.stderr"
  status=$?
  set -e
  printf '%s\n' "$status" > "$capture_root/first-structure.status"
  test "$status" -eq 0 || exit "$status"
 fi
done
set +e
nice -n 10 node "$fixture/audit-shifted-width-consumer.cjs" "$capture_root" > "$capture_root/audit.json" 2> "$capture_root/audit.stderr"
status=$?
set -e
printf '%s\n' "$status" > "$capture_root/audit.status"
date -u +%FT%TZ > "$capture_root/finished.utc"
exit "$status"
