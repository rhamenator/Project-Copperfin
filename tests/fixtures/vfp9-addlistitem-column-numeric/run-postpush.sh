#!/usr/bin/env bash
# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only
# Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
# VR-5611-ADDLISTITEM-COLUMN-NUMERIC-001: exact signed pushed-head full95.
set -u
cd /home/rich/.codex/worktrees/addlistitem-column-numeric-5611/Project-Copperfin || exit 2
evidence=build/addlistitem-column-native
fixture=tests/fixtures/vfp9-addlistitem-column-numeric
expected_head="${1:?Supply exact signed pushed head}"
[ "$(git rev-parse HEAD)" = "$expected_head" ] || exit 3
[ "$(git rev-parse origin/fix/addlistitem-column-numeric-5611)" = "$expected_head" ] || exit 3
[ "$(git log -1 --format=%G?)" = G ] || exit 3
[ "$(< "$evidence/fixed-complete-audit.status")" = 0 ] || exit 3
[ "$(< "$evidence/after-ci-main-contracts.status")" = 0 ] || exit 3
[ "$(< "$evidence/after-ci-main-manifest-delta-audit-v2.status")" = 0 ] || exit 3
git rev-parse HEAD > "$evidence/postpush-head.before"
sha256sum -c "$fixture/fixed-inputs.sha256" > "$evidence/postpush-source259-before.log" || exit 4
regex='^(test_native_test_isolation_contract|test_prg_engine_((addlistitem_column|additem_column|additem|addlistitem|removelistitem|removeitem|topitemid|topindex|listitemid|listindex|list_index|listitem|itemidtoindex|indextoitemid|selectedid|selected_index|itemdata_index|getcolor_default)_numeric(_.*)?|numeric_behavior|int64_exact_expression|runtime_surface_functions|runtime_surface_itemid_selectors))$'
ctest --test-dir build/addlistitem-column-gcc -N -R "$regex" > "$evidence/postpush-test-list.log" 2>&1 || exit 5
rg -q 'Total Tests: 95$' "$evidence/postpush-test-list.log" || exit 5
ctest --test-dir build/addlistitem-column-gcc --show-only=json-v1 -R "$regex" > "$evidence/postpush-test-metadata.json" 2> "$evidence/postpush-test-metadata.stderr" || exit 5
node "$fixture/audit-test-metadata.cjs" "$evidence/postpush-test-metadata.json" > "$evidence/postpush-metadata-audit.jsonl" 2> "$evidence/postpush-metadata-audit.stderr" || exit 5
# The exact frozen runtime sources match the already-built GNU fixed95 library;
# no runtime source, assertion, selector, compiler flag or timeout changed at sync.
nice -n 10 ionice -c 2 -n 7 ctest --test-dir build/addlistitem-column-gcc -V -R "$regex" --parallel 1 --timeout 700 > "$evidence/postpush.log" 2>&1
result=$?
printf '%s\n' "$result" > "$evidence/postpush.status"
date -u +%Y-%m-%dT%H:%M:%SZ > "$evidence/postpush.completed-at"
sha256sum "$evidence/postpush.log" > "$evidence/postpush.log.sha256"
sha256sum -c "$fixture/fixed-inputs.sha256" > "$evidence/postpush-source259-after.log" || exit 6
git rev-parse HEAD > "$evidence/postpush-head.after"
cmp "$evidence/postpush-head.before" "$evidence/postpush-head.after" || exit 6
[ "$(git rev-parse HEAD)" = "$expected_head" ] || exit 6
exit "$result"
