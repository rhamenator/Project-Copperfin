#!/bin/bash
# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only
# Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
# RQ-CF-PRG-NUMERIC-DEFAULT-TRANSFORM-001: offline receipts, no native execution.
set -euo pipefail
cd /home/rich/.codex/worktrees/numeric-literal-display-5611/Project-Copperfin
out=${1-}
mode=${2-}
[[ "$mode" = '' || "$mode" = '--combined' ]] || exit 93
analysis=${3-}
[[ "$analysis" = '' || "$analysis" = '--cells' ]] || exit 93
[[ "$out" =~ ^build/default-transform-str-crt-coverage[0-9]+$ ]] || exit 93
test ! -e "$out" || exit 96
mkdir "$out"
fixture=tests/fixtures/vfp9-numeric-default-transform
git rev-parse HEAD > "$out/source-commit.txt"
git status --porcelain > "$out/source-status.txt"
sha256sum "$fixture"/*.cjs "$fixture"/*.json "$fixture"/*.prg \
 "$fixture"/str-middle-captures1/* "$fixture"/str-fixed-captures1/* \
 "$fixture"/gcvt-captures1/* "$fixture"/crt-nativeonly-captures1/* \
 "$fixture"/crt-precision-captures1/* \
 "$fixture/str-crt-coverage-check.sh" > "$out/inputs.sha256"
stage() {
 local name=$1; shift
 sha256sum -c "$out/inputs.sha256" > "$out/$name.inputs-before.txt"
 set +e
 pgrep -a -x 'wine|wine64|wine-preloader|wineserver|wineserver64|vfp9.exe' > "$out/$name.prefix.txt"
 local idle=$?
 set -e
 printf '%s\n' "$idle" > "$out/$name.prefix.status"
 test "$idle" -eq 1 || exit 76
 cat /proc/pressure/io > "$out/$name.pressure-before.txt"
 awk '$1=="full" {split($2,a,"=");split($3,b,"=");found=1;if(a[2]>=1||b[2]>=1)exit 75} END {if(!found)exit 75}' "$out/$name.pressure-before.txt"
 set +e
 nice -n 10 "$@" > "$out/$name.out" 2> "$out/$name.stderr"
 local status=$?
 set -e
 printf '%s\n' "$status" > "$out/$name.status"
 test "$status" -eq 0 || exit "$status"
 test ! -s "$out/$name.stderr" || exit 98
 sha256sum -c "$out/inputs.sha256" > "$out/$name.inputs-after.txt"
}
stage coverage-tests node "$fixture/test-inventory-str-crt-coverage.cjs"
stage loader-tests node "$fixture/test-load-str-crt-coverage.cjs"
if [ "$mode" = '--combined' ]; then
 stage precision-join-tests node "$fixture/test-str-crt-precision-join.cjs"
 stage precision-reader node "$fixture/audit-crt-precision.cjs" "$fixture/crt-precision-captures1"
fi
stage str-reader node "$fixture/audit-str-middle.cjs" "$fixture/str-middle-captures1"
stage crt-reader node "$fixture/audit-crt.cjs" "$fixture/crt-nativeonly-captures1"
stage overlap-prerequisite node "$fixture/analyze-str-crt-overlap.cjs"
if [ "$mode" = '--combined' ]; then
 stage inventory node "$fixture/load-str-crt-coverage.cjs" --combined
 stage repeat node "$fixture/load-str-crt-coverage.cjs" --combined
else
 stage inventory node "$fixture/load-str-crt-coverage.cjs"
 stage repeat node "$fixture/load-str-crt-coverage.cjs"
fi
stage readback cmp "$out/inventory.out" "$out/repeat.out"
if [ "$analysis" = '--cells' ]; then
 stage cell-metric-tests node "$fixture/test-compare-str-crt-cell.cjs"
 stage cell-analysis node "$fixture/analyze-str-crt-cells.cjs"
 stage cell-repeat node "$fixture/analyze-str-crt-cells.cjs"
 stage cell-bytecompare cmp "$out/cell-analysis.out" "$out/cell-repeat.out"
fi
printf 'Offline coverage stages complete; no native invoked\n'
