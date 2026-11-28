#!/usr/bin/env bash
# One-shot correction after focused v3 exposed assigned real metadata on the
# runtime divisor at nonzero DECIMALS. Earlier manifests/receipts stay immutable.
set -euo pipefail
cd /home/rich/.codex/worktrees/listitem-column-read-5611/Project-Copperfin
fixture=tests/fixtures/vfp9-listitem-column-numeric
manifest="$fixture/broader-inputs-v4.sha256"
[ ! -e "$manifest" ] || { echo "broader v4 manifest already exists" >&2; exit 2; }
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
  "$fixture/freeze-broader-inputs-v3.sh" \
  "$fixture/freeze-broader-inputs-v4.sh" \
  "$fixture/run-broader.sh" >> "$paths"
sort -u "$paths" -o "$paths"
while IFS= read -r file; do sha256sum "$file"; done < "$paths" > "$manifest"
echo "broader v4 input manifest entries=$(wc -l < "$manifest")"
