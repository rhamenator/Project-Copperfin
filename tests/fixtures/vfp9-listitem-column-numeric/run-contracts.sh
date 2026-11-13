#!/usr/bin/env bash
# RQ-CF-PRG-LISTITEM-COLUMN-NUMERIC-001 / DQ-DV-5611-LISTITEM-COLUMN-NUMERIC-001.
# Run only after both fixed106, hostile and SAME-compiler preservation gates complete.
set -u
root=/home/rich/.codex/worktrees/listitem-column-read-5611/Project-Copperfin
cd "$root" || exit 2
fixture=tests/fixtures/vfp9-listitem-column-numeric
evidence=build/listitem-column-native
contracts="$evidence/contracts"
mkdir -p "$contracts/tmp" || exit 2
for gate in \
  fixed/fixed-compile-gcc fixed/fixed-gcc \
  fixed/fixed-compile-clang fixed/fixed-clang fixed/complete-audit; do
  [ "$(< "$evidence/$gate.status")" = 0 ] || exit 3
done
for compiler in gcc clang; do
  for mode in COPPERFIN VFP9; do
    for session in 1 2; do
      [ "$(< "$evidence/hostile-$compiler-$mode-$session.status")" = 0 ] || exit 3
    done
  done
  for phase in baseline fixed; do
    for gate in compile '' audit; do
      suffix=${gate:+-$gate}
      [ "$(< "$evidence/preservation/preservation-$phase-$compiler$suffix.status")" = 0 ] || exit 3
    done
  done
  [ "$(< "$evidence/preservation/preservation-fixed-$compiler.cmp.status")" = 0 ] || exit 3
done
sha256sum -c "$fixture/fixed-inputs.sha256" > "$contracts/fixed-inputs-before.log" || exit 4
sha256sum -c "$fixture/preservation-inputs.sha256" > "$contracts/preservation-inputs-before.log" || exit 4
regex='^test_(contributor_signoff_contract|changelog_fragment_assembler|changelog_fragments_valid|agent_channel_contract|safety_traceability_workflow_contract)$'
ctest --test-dir build/listitem-column-gcc -N -R "$regex" > "$contracts/test-list.log" 2>&1 || exit 5
rg -q 'Total Tests: 5$' "$contracts/test-list.log" || exit 5
export TMPDIR="$root/$contracts/tmp"
nice -n 10 ionice -c 2 -n 7 ctest --test-dir build/listitem-column-gcc \
  -V -R "$regex" --parallel 1 > "$contracts/development-contracts.log" 2>&1
result=$?
printf '%s\n' "$result" > "$contracts/development-contracts.status"
date -u +%Y-%m-%dT%H:%M:%SZ > "$contracts/development-contracts.completed-at"
sha256sum "$contracts/development-contracts.log" > "$contracts/development-contracts.log.sha256"
printf '%s\n' "$(git rev-parse HEAD)" > "$contracts/product-head"
sha256sum -c "$fixture/fixed-inputs.sha256" > "$contracts/fixed-inputs-after.log" || exit 6
sha256sum -c "$fixture/preservation-inputs.sha256" > "$contracts/preservation-inputs-after.log" || exit 6
exit "$result"
