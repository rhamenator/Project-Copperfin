#!/usr/bin/env bash
# One-shot review-fix preservation freeze. The original manifest and receipts
# remain immutable historical evidence.
set -euo pipefail
cd /home/rich/.codex/worktrees/listitem-column-read-5611/Project-Copperfin
fixture=tests/fixtures/vfp9-listitem-column-numeric
manifest="$fixture/preservation-inputs-v2.sha256"
[ ! -e "$manifest" ] || { echo "preservation v2 manifest already exists" >&2; exit 2; }
for file in \
  "$fixture/preservation-plan.md" \
  "$fixture/excluded-scope-preservation.cpp" \
  "$fixture/audit-preservation.cjs" \
  "$fixture/run-preservation.sh" \
  "$fixture/freeze-preservation-inputs-v2.sh"; do
  sha256sum "$file"
done > "$manifest"
echo "preservation v2 input manifest entries=$(wc -l < "$manifest")"
