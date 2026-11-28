#!/usr/bin/env bash
# One-shot correction after attempt1 named a CTest-only script as a build target.
# The failed attempt1 receipts and v1 manifest remain retained and immutable.
set -euo pipefail
cd /home/rich/.codex/worktrees/listitem-column-read-5611/Project-Copperfin
fixture=tests/fixtures/vfp9-listitem-column-numeric
manifest="$fixture/broader-inputs-v2.sha256"
[ ! -e "$manifest" ] || { echo "broader v2 manifest already exists" >&2; exit 2; }
paths=$(mktemp)
trap 'rm -f "$paths"' EXIT
git ls-files \
  'CMakeLists.txt' \
  'cmake/**' \
  'include/copperfin/runtime/**' \
  'src/runtime/**' \
  'tests/CMakeLists.txt' \
  'tests/CopperfinTestIsolation.cmake' \
  'tests/prg_engine_test_support.*' \
  'tests/test_prg_engine*' \
  'tests/test_prg_numeric*' > "$paths"
printf '%s\n' \
  "$fixture/audit-test-metadata.cjs" \
  "$fixture/freeze-broader-inputs.sh" \
  "$fixture/freeze-broader-inputs-v2.sh" \
  "$fixture/run-broader.sh" >> "$paths"
sort -u "$paths" -o "$paths"
while IFS= read -r file; do sha256sum "$file"; done < "$paths" > "$manifest"
echo "broader v2 input manifest entries=$(wc -l < "$manifest")"
