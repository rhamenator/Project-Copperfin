#!/usr/bin/env bash
# RQ-CF-PRG-ADDLISTITEM-COLUMN-NUMERIC-001 / DQ-DV-5611-ADDLISTITEM-COLUMN-NUMERIC-001.
# Run ONLY after BOTH completed fixed95 and hostile/preservation gates.
set -u
cd /home/rich/.codex/worktrees/addlistitem-column-numeric-5611/Project-Copperfin || exit 2
evidence=build/addlistitem-column-native
for gate in fixed-driver fixed-complete-audit preservation-fixed-gcc-driver preservation-fixed-clang-driver preservation-fixed-gcc.cmp preservation-fixed-clang.cmp preservation-fixed-gcc-audit preservation-fixed-clang-audit hostile-clang-driver wiring-scope-audit; do
 [ "$(< "$evidence/$gate.status")" = 0 ] || exit 3
done
for compiler in gcc clang; do
 for mode in COPPERFIN VFP9; do
  for session in 1 2; do
   [ "$(< "$evidence/hostile-$compiler-$mode-$session.status")" = 0 ] || exit 3
  done
 done
done
sha256sum -c "$evidence/fixed-inputs.sha256" > "$evidence/contracts-inputs-before.log" || exit 4
regex='^test_(contributor_signoff_contract|changelog_fragment_assembler|changelog_fragments_valid|agent_channel_contract|safety_traceability_workflow_contract)$'
ctest --test-dir build/addlistitem-column-gcc -N -R "$regex" > "$evidence/contracts-test-list.log" 2>&1 || exit 5
rg -q 'Total Tests: 5$' "$evidence/contracts-test-list.log" || exit 5
export TMPDIR=/home/rich/.codex/worktrees/addlistitem-column-numeric-5611/Project-Copperfin/build/addlistitem-column-native/tmp
nice -n 10 ionice -c 2 -n 7 ctest --test-dir build/addlistitem-column-gcc -V -R "$regex" --parallel 1 > "$evidence/development-contracts.log" 2>&1
result=$?
printf '%s\n' "$result" > "$evidence/development-contracts.status"
date -u +%Y-%m-%dT%H:%M:%SZ > "$evidence/development-contracts.completed-at"
sha256sum "$evidence/development-contracts.log" > "$evidence/development-contracts.log.sha256"
sha256sum -c "$evidence/fixed-inputs.sha256" > "$evidence/contracts-inputs-after.log" || exit 6
exit "$result"
