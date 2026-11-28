#!/usr/bin/env bash
# One-shot post-wiring fixed-input manifest creation. Never refresh after fixed execution starts.
set -euo pipefail
cd /home/rich/.codex/worktrees/listitem-column-read-5611/Project-Copperfin
fixture=tests/fixtures/vfp9-listitem-column-numeric
manifest="$fixture/fixed-inputs.sha256"
[ ! -e "$manifest" ] || { echo "fixed manifest already exists" >&2; exit 2; }
paths=$(mktemp)
trap 'rm -f "$paths"' EXIT
sed -n 's/^[a-f0-9]\{64\}  //p' "$fixture/frozen-original-inputs.sha256" > "$paths"
printf '%s\n' src/runtime/prg_numeric_constant_arithmetic_metadata.h \
 tests/test_prg_numeric_constant_arithmetic_metadata_helper.cpp \
 "$fixture/freeze-fixed-inputs.sh" "$fixture/run-fixed.sh" \
 "$fixture/audit-fixed-complete.cjs" >> "$paths"
sort -u "$paths" -o "$paths"
while IFS= read -r file; do sha256sum "$file"; done < "$paths" > "$manifest"
echo "fixed input manifest entries=$(wc -l < "$manifest")"
