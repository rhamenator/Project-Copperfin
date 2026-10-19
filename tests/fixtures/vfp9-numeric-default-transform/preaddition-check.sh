#!/bin/bash
# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only
# Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
# Offline AFTER-capture checks only; never grants a compiler/native/VM window.
set -eu
cd /home/rich/.codex/worktrees/numeric-literal-display-5611/Project-Copperfin
check_root=${1-}
[[ "$check_root" =~ ^build/default-transform-preaddition[0-9]+$ ]] || exit 93
test ! -e "$check_root" || exit 96
mkdir "$check_root"
fixture=tests/fixtures/vfp9-numeric-default-transform
git rev-parse HEAD > "$check_root/source-base-commit.txt"
git status --porcelain > "$check_root/source-status.txt"
cp "$fixture/compare-preaddition-operands.cjs" "$check_root/source-compare-preaddition-operands.cjs"
cp "$fixture/preaddition-check.sh" "$check_root/source-preaddition-check.sh"
date -u +%FT%TZ > "$check_root/started.utc"
sha256sum "$fixture"/{compare-preaddition-operands.cjs,preaddition-check.sh,compare-collapsed-stage-operands.cjs,join-observable-stage-adjacency.cjs,classify-graph-rejections.cjs,inventory-adjacent-transitions.cjs,decompose-round-observations.cjs,arithmetic-inline-cases.json} \
 "$fixture"/{collapsed-operands-analysis1,stage-adjacency-analysis1,adjacent-transition-inventory1,graph-rejection-classification1,round-decomposition-analysis1}/{manifest.json,SHA256SUMS,*.gz} \
 "$fixture"/{round-inline-captures1,arithmetic-inline-captures1,str-middle-captures1}/{SHA256SUMS,*.gz} > "$check_root/inputs.sha256"
stage() {
 local name=$1
 shift
 sha256sum -c "$check_root/inputs.sha256" > "$check_root/$name.inputs-before.txt"
 set +e
 pgrep -a -x 'wine|wine64|wine-preloader|wineserver|wineserver64|vfp9.exe' > "$check_root/$name.prefix.txt"
 local prefix_status=$?
 set -e
 printf '%s\n' "$prefix_status" > "$check_root/$name.prefix.status"
 test "$prefix_status" -eq 1 || return 76
 cat /proc/pressure/io > "$check_root/$name.pressure-before.txt"
 set +e
 awk '$1=="full" {split($2,a,"=");split($3,b,"=");found=1;if(a[2]>=1||b[2]>=1)exit 75} END {if(!found)exit 75}' "$check_root/$name.pressure-before.txt"
 local guard_status=$?
 set -e
 printf '%s\n' "$guard_status" > "$check_root/$name.guard.status"
 test "$guard_status" -eq 0 || return 75
 set +e
 nice -n 10 "$@" > "$check_root/$name.json" 2> "$check_root/$name.stderr"
 local status=$?
 set -e
 printf '%s\n' "$status" > "$check_root/$name.status"
 test "$status" -eq 0 || return "$status"
 test ! -s "$check_root/$name.stderr"
 sha256sum -c "$check_root/inputs.sha256" > "$check_root/$name.inputs-after.txt"
}
stage syntax node --check "$fixture/compare-preaddition-operands.cjs"
stage shell-syntax bash -n "$fixture/preaddition-check.sh"
stage analysis node "$fixture/compare-preaddition-operands.cjs"
stage readback node "$fixture/compare-preaddition-operands.cjs"
cmp "$check_root/analysis.json" "$check_root/readback.json"
# Replay the immutable BEFORE snapshot using its original path for bound inputs.
stage snapshot-replay node -e 'const fs=require("node:fs"),M=require("node:module"),p=require("node:path");const f=p.resolve(process.argv[1]),m=new M(f);m.filename=f;m.paths=M._nodeModulePaths(p.dirname(f));m._compile(fs.readFileSync(process.argv[2],"utf8"),f);' "$fixture/compare-preaddition-operands.cjs" "$check_root/source-compare-preaddition-operands.cjs"
cmp "$check_root/analysis.json" "$check_root/snapshot-replay.json"
date -u +%FT%TZ > "$check_root/finished.utc"
