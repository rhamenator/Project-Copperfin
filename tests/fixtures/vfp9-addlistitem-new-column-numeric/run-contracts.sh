#!/usr/bin/env bash
# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only
# Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
# RQ-CF-PRG-ADDLISTITEM-NEW-COLUMN-NUMERIC-001 / DQ-DV001.
set -u
cd /home/rich/.codex/worktrees/addlistitem-new-column-numeric-5611/Project-Copperfin || exit 2
fixture=tests/fixtures/vfp9-addlistitem-new-column-numeric
evidence=build/addlistitem-new-column-native
for gate in fixed-driver hostile-driver preservation-fixed-gcc-driver preservation-fixed-clang-driver preservation-fixed-gcc.cmp preservation-fixed-clang.cmp preservation-fixed-gcc-audit preservation-fixed-clang-audit; do
 [ "$(< "$evidence/$gate.status")" = 0 ] || exit 3
done
[ "$(< "$evidence/fixed/complete-audit.status")" = 0 ] || exit 3
sha256sum -c "$fixture/fixed-inputs.sha256" > "$evidence/contracts-inputs-before.log" || exit 4
regex='^test_(contributor_signoff_contract|changelog_fragment_assembler|changelog_fragments_valid|agent_channel_contract|safety_traceability_workflow_contract)$'
ctest --test-dir build/addlistitem-new-column-gcc -N -R "$regex" > "$evidence/contracts-test-list.log" 2>&1 || exit 5
rg -q 'Total Tests: 5$' "$evidence/contracts-test-list.log" || exit 5
mkdir -p "$evidence/contracts-tmp"
chmod 700 "$evidence/contracts-tmp"
export TMPDIR=/home/rich/.codex/worktrees/addlistitem-new-column-numeric-5611/Project-Copperfin/build/addlistitem-new-column-native/contracts-tmp
nice -n 10 ionice -c 2 -n 7 ctest --test-dir build/addlistitem-new-column-gcc -V -R "$regex" --parallel 1 > "$evidence/development-contracts.log" 2>&1
result=$?
printf '%s\n' "$result" > "$evidence/development-contracts.status"
date -u +%Y-%m-%dT%H:%M:%SZ > "$evidence/development-contracts.completed-at"
sha256sum "$evidence/development-contracts.log" > "$evidence/development-contracts.log.sha256"
sha256sum -c "$fixture/fixed-inputs.sha256" > "$evidence/contracts-inputs-after.log" || exit 6
exit "$result"
