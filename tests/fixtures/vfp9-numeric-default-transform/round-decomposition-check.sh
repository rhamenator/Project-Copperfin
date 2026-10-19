#!/bin/bash
# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only
# Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
# Offline AFTER-capture analysis only. Never grants a native/compiler/VM window.
set -eu
cd /home/rich/.codex/worktrees/numeric-literal-display-5611/Project-Copperfin
capture_root=${1-}
[[ "$capture_root" =~ ^build/default-transform-round-decomposition[0-9]+$ ]] || exit 93
test ! -e "$capture_root" || exit 96
mkdir "$capture_root"
fixture=tests/fixtures/vfp9-numeric-default-transform
git rev-parse HEAD > "$capture_root/source-base-commit.txt"
git status --porcelain > "$capture_root/source-status.txt"
cp "$fixture/decompose-round-observations.cjs" "$capture_root/source-decompose-round-observations.cjs"
cp "$fixture/round-decomposition-check.sh" "$capture_root/source-round-decomposition-check.sh"
date -u +%FT%TZ > "$capture_root/started.utc"
sha256sum "$fixture"/{decompose-round-observations.cjs,round-decomposition-check.sh} \
 "$fixture"/{round-inline-captures1,arithmetic-inline-captures1,str-middle-captures1}/{audit.json.gz,first-results.txt.gz,repeat-results.txt.gz,SHA256SUMS} > "$capture_root/inputs.sha256"
for name in decomposition readback; do
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
 nice -n 10 node "$fixture/decompose-round-observations.cjs" > "$capture_root/$name.json" 2> "$capture_root/$name.stderr"
 status=$?
 set -e
 printf '%s\n' "$status" > "$capture_root/$name.status"
 test "$status" -eq 0 || exit "$status"
 sha256sum -c "$capture_root/inputs.sha256" > "$capture_root/$name.inputs-check.txt"
done
cmp "$capture_root/decomposition.json" "$capture_root/readback.json"
date -u +%FT%TZ > "$capture_root/finished.utc"
