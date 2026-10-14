#!/usr/bin/env bash
# RQ-CF-PRG-ADDITEM-COLUMN-NUMERIC-001 / DQ-DV-5611-ADDITEM-COLUMN-NUMERIC-001.
# Run only after BOTH fixed90/hostile/preservation audits complete.
set -u
cd /home/rich/.codex/worktrees/additem-column-numeric-5611/Project-Copperfin || exit 2
evidence=build/additem-column-native
for gate in fixed-driver fixed-gcc-complete-audit fixed-clang-complete-audit; do
 [ "$(< "$evidence/$gate.status")" = 0 ] || exit 3
done
sha256sum -c "$evidence/fixed-inputs.sha256" > "$evidence/contracts-inputs-before.log" || exit 4
regex='^test_(contributor_signoff_contract|changelog_fragment_assembler|changelog_fragments_valid|agent_channel_contract|safety_traceability_workflow_contract)$'
ctest --test-dir build/additem-column-gcc -N -R "$regex" > "$evidence/contracts-test-list.log" 2>&1
rg -q 'Total Tests: 5$' "$evidence/contracts-test-list.log" || exit 5
export TMPDIR=/home/rich/.codex/worktrees/additem-column-numeric-5611/Project-Copperfin/build/additem-column-native/tmp
nice -n 10 ionice -c 2 -n 7 ctest --test-dir build/additem-column-gcc -V -R "$regex" --parallel 1 > "$evidence/development-contracts.log" 2>&1
result=$?
printf '%s\n' "$result" > "$evidence/development-contracts.status"
date -u +%Y-%m-%dT%H:%M:%SZ > "$evidence/development-contracts.completed-at"
sha256sum "$evidence/development-contracts.log" > "$evidence/development-contracts.log.sha256"
sha256sum -c "$evidence/fixed-inputs.sha256" > "$evidence/contracts-inputs-after.log" || exit 6
exit "$result"
