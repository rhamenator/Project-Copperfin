#!/bin/bash
# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only
# Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
# RQ-CF-PRG-NUMERIC-DEFAULT-TRANSFORM-001: offline source qualification only.
set -euo pipefail
cd /home/rich/.codex/worktrees/numeric-literal-display-5611/Project-Copperfin
mode=${1-}
check_root=${2-}
case "$mode:$check_root" in
 generate:build/default-transform-original-scale-middle-generate[0-9]*|check:build/default-transform-original-scale-middle-source-checks[0-9]*) ;;
 *) exit 93 ;;
esac
[[ "$check_root" =~ ^build/default-transform-original-scale-middle-(generate|source-checks)[0-9]+$ ]] || exit 93
test ! -e "$check_root" || exit 96
mkdir "$check_root"
fixture=tests/fixtures/vfp9-numeric-default-transform
git rev-parse HEAD > "$check_root/pre-check-source-commit.txt"
git status --porcelain > "$check_root/pre-check-source-status.txt"
names=(emit-original-scale-middle-consumer.cjs audit-original-scale-middle-consumer.cjs check-original-scale-middle-reader.cjs check-original-scale-middle-worker.cjs original-scale-middle-consumer-capture.sh original-scale-middle-source-check.sh)
if [ "$mode" = check ]; then names+=(original-scale-middle-consumer.prg original-scale-middle-consumer-cases.json); fi
for name in "${names[@]}"; do cp "$fixture/$name" "$check_root/source-$name"; done
# Bind original-parent evidence only; shifted/arithmetic captures are not inputs.
(
 for name in "${names[@]}"; do sha256sum "$fixture/$name"; done
 sha256sum "$fixture"/{round-inline.prg,str-middle-cases.json}
 sha256sum "$fixture"/round-inline-captures1/{SHA256SUMS,audit.json.gz,first-results.txt.gz,repeat-results.txt.gz,first.out.gz,repeat.out.gz,first.status.gz,repeat.status.gz,first.stderr.gz,repeat.stderr.gz,source-round-inline.prg.gz}
 sha256sum "$fixture"/gcvt-captures1/source-{gcvt-probe.c,gcvt-boundary.h,crt-probe.c,crt-binding.h}.gz
) | sort -u > "$check_root/inputs.sha256"
stage() {
 local name=$1
 shift
 sha256sum -c "$check_root/inputs.sha256" > "$check_root/$name.inputs-before.txt"
 set +e
 pgrep -a -x 'wine|wine64|wine-preloader|wineserver|wineserver64|vfp9.exe' > "$check_root/$name.prefix.txt"
 local prefix_status=$?
 set -e
 printf '%s\n' "$prefix_status" > "$check_root/$name.prefix.status"
 test "$prefix_status" -eq 1 || exit 76
 cat /proc/pressure/io > "$check_root/$name.pressure-before.txt"
 set +e
 awk '$1=="full" {split($2,a,"=");split($3,b,"=");found=1;if(a[2]>=1||b[2]>=1)exit 75} END {if(!found)exit 75}' "$check_root/$name.pressure-before.txt"
 local guard_status=$?
 set -e
 printf '%s\n' "$guard_status" > "$check_root/$name.guard.status"
 test "$guard_status" -eq 0 || exit 75
 set +e
 nice -n 10 "$@" > "$check_root/$name.out" 2> "$check_root/$name.stderr"
 local status=$?
 set -e
 printf '%s\n' "$status" > "$check_root/$name.status"
 test "$status" -eq 0 || exit "$status"
 test ! -s "$check_root/$name.stderr" || exit 98
 sha256sum -c "$check_root/inputs.sha256" > "$check_root/$name.inputs-after.txt"
}
if [ "$mode" = generate ]; then
 stage generate-source node "$fixture/emit-original-scale-middle-consumer.cjs" source
 stage generate-cases node "$fixture/emit-original-scale-middle-consumer.cjs" cases
else
 stage syntax-emitter node --check "$fixture/emit-original-scale-middle-consumer.cjs"
 stage syntax-reader node --check "$fixture/audit-original-scale-middle-consumer.cjs"
 stage syntax-reader-tests node --check "$fixture/check-original-scale-middle-reader.cjs"
 stage syntax-worker-tests node --check "$fixture/check-original-scale-middle-worker.cjs"
 stage syntax-worker bash -n "$fixture/original-scale-middle-consumer-capture.sh"
 stage syntax-offline-worker bash -n "$fixture/original-scale-middle-source-check.sh"
 stage regenerate-source node "$fixture/emit-original-scale-middle-consumer.cjs" source
 stage regenerate-cases node "$fixture/emit-original-scale-middle-consumer.cjs" cases
 stage compare-source cmp "$fixture/original-scale-middle-consumer.prg" "$check_root/regenerate-source.out"
 stage compare-cases cmp "$fixture/original-scale-middle-consumer-cases.json" "$check_root/regenerate-cases.out"
 stage source-reader node "$fixture/audit-original-scale-middle-consumer.cjs" --source-only
 stage synthetic-reader node "$fixture/check-original-scale-middle-reader.cjs"
 stage synthetic-worker node "$fixture/check-original-scale-middle-worker.cjs"
fi
printf '%s offline source stages complete; no native invoked\n' "$mode"
