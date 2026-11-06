#!/usr/bin/env bash
# One-shot pre-wire inventory creation. Re-running after wiring is forbidden.
set -euo pipefail
cd "$(git rev-parse --show-toplevel)"
fixture=tests/fixtures/vfp9-listitem-column-numeric
paths=$(mktemp)
trap 'rm -f "$paths"' EXIT
{
    printf '%s\n' CMakeLists.txt tests/CMakeLists.txt tests/CopperfinTestIsolation.cmake
    git ls-files 'include/copperfin/runtime/*' 'src/runtime/prg_engine*'
    git ls-files 'tests/test_prg_engine*' 'tests/prg_engine_test_support.*'
    printf '%s\n' \
        "$fixture/audit-originals-complete.cjs" \
        "$fixture/audit-test-metadata.cjs" \
        "$fixture/freeze-original-inputs.sh" \
        "$fixture/pre-freeze-original106-test-names.txt" \
        "$fixture/run-originals.sh"
} | LC_ALL=C sort -u > "$paths"
while IFS= read -r file; do
    test -f "$file" || { printf 'Missing frozen input: %s\n' "$file" >&2; exit 2; }
    sha256sum "$file"
done < "$paths" > "$fixture/frozen-original-inputs.sha256"
wc -l "$fixture/frozen-original-inputs.sha256"
