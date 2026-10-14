#!/usr/bin/env bash
# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only
# Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
# RQ-CF-PRG-ADDLISTITEM-NEW-COLUMN-NUMERIC-001 / VR001+002.
# Actual exact pushed HEAD100, never substitute earlier local/head evidence.
set -u
cd /home/rich/.codex/worktrees/addlistitem-new-column-numeric-5611/Project-Copperfin || exit 2
[ "$#" = 1 ] || exit 2
expected="$1"
fixture=tests/fixtures/vfp9-addlistitem-new-column-numeric
evidence=build/addlistitem-new-column-native/postpush
mkdir -p "$evidence/tmp"
chmod 700 "$evidence/tmp"
git rev-parse HEAD > "$evidence/head-before.txt"
[ "$(< "$evidence/head-before.txt")" = "$expected" ] || exit 3
[ "$(git rev-parse origin/fix/addlistitem-new-column-numeric-5611)" = "$expected" ] || exit 3
git diff --quiet && git diff --cached --quiet || exit 3
sha256sum -c "$fixture/fixed-inputs.sha256" > "$evidence/inputs-before.log" || exit 4
export TMPDIR=/home/rich/.codex/worktrees/addlistitem-new-column-numeric-5611/Project-Copperfin/build/addlistitem-new-column-native/postpush/tmp
export COPPERFIN_LOCALE=en-US
regex='^(test_native_test_isolation_contract|test_prg_engine_((addlistitem_new_column|addlistitem_column|additem_column|additem|addlistitem|removelistitem|removeitem|topitemid|topindex|listitemid|listindex|list_index|listitem|itemidtoindex|indextoitemid|selectedid|selected_index|itemdata_index|getcolor_default)_numeric(_.*)?|numeric_behavior|int64_exact_expression|runtime_surface_functions|runtime_surface_itemid_selectors))$'
ctest --test-dir build/addlistitem-new-column-gcc -N -R "$regex" > "$evidence/test-list.log" 2>&1 || exit 5
rg -q 'Total Tests: 100$' "$evidence/test-list.log" || exit 5
ctest --test-dir build/addlistitem-new-column-gcc --show-only=json-v1 -R "$regex" > "$evidence/test-metadata.json" 2> "$evidence/test-metadata.stderr" || exit 5
node "$fixture/audit-test-metadata.cjs" "$evidence/test-metadata.json" > "$evidence/metadata-audit.jsonl" 2> "$evidence/metadata-audit.stderr" || exit 5
nice -n 10 ionice -c 2 -n 7 ctest --test-dir build/addlistitem-new-column-gcc -V -R "$regex" --parallel 1 --timeout 700 > "$evidence/full.log" 2>&1
result=$?
printf '%s\n' "$result" > "$evidence/full.status"
date -u +%Y-%m-%dT%H:%M:%SZ > "$evidence/completed-at.txt"
sha256sum "$evidence/full.log" > "$evidence/full.log.sha256"
[ "$result" = 0 ] || exit 6
node - "$evidence/full.log" <<'JS' > "$evidence/counter-audit.jsonl" 2> "$evidence/counter-audit.stderr"
const fs=require('fs'),{auditRaw}=require('./tests/fixtures/vfp9-addlistitem-new-column-numeric/audit-fixed-complete.cjs');
console.log(JSON.stringify(auditRaw(fs.readFileSync(process.argv[2],'utf8'))));
JS
result=$?
printf '%s\n' "$result" > "$evidence/counter-audit.status"
[ "$result" = 0 ] || exit 7
sha256sum -c "$fixture/fixed-inputs.sha256" > "$evidence/inputs-after.log" || exit 8
git rev-parse HEAD > "$evidence/head-after.txt"
[ "$(< "$evidence/head-after.txt")" = "$expected" ] || exit 9
git diff --quiet && git diff --cached --quiet || exit 9
printf 'Exact pushed HEAD100 complete0; allprior95/192-four432/source270/commands verified.\n'
