# DBF Header Parser Robustness Testing

## Status

This is **optional defense-in-depth testing**, not a v1 or RC release gate.
It does not block any build, workflow, or evaluation candidate, and it does
not claim exhaustive format or security coverage.

## Purpose

`copperfin::vfp::parse_dbf_header` and `parse_dbf_header_from_file`
(`src/vfp/dbf_header.cpp`) parse the fixed 32-byte prefix of a `.dbf` file
that Copperfin treats as untrusted input by default -- table files can arrive
from another program, another era of VFP tooling, or a partially-corrupted
copy. `tests/test_dbf_header_robustness.cpp` adds bounded, deterministic
robustness coverage on top of the existing functional tests in
`tests/test_vfp_assets.cpp` (`test_parse_dbf_header`,
`test_parse_dbf_header_rejects_short_input`), which check correctness for a
handful of hand-picked inputs but do not systematically sweep malformed or
boundary-value input.

## Scope

In scope: `parse_dbf_header` and `parse_dbf_header_from_file`, and the
`DbfHeader` accessor methods reachable from a successful parse
(`looks_like_dbf`, `has_database_container`, `has_production_index`,
`has_structural_cdx`, `has_memo_file`, `version_description`,
`last_update_iso8601`) plus `dbf_code_page_from_mark`.

Out of scope, deliberately: the DBF field-descriptor array and record data
that follow the header (`src/vfp/dbf_table.cpp`), CDX/IDX/NDX/MDX index
parsing (`src/vfp/cdx_header.cpp`, `src/vfp/index_probe.cpp`), and memo file
parsing. These are separate, larger parsers with their own boundary
conditions; hardening them is natural follow-on work but is not attempted
here. This slice targets only Copperfin's own code -- it does not target any
third-party system, download any external corpus, or use network access.

## What the test does

`tests/test_dbf_header_robustness.cpp` is a small, self-contained CTest
executable with five fixed, bounded sweeps, all built from synthetic local
byte sequences constructed in-process (no external corpus, no filesystem
inputs beyond files the test itself writes and removes):

1. **Boundary-size sweep**: every input length from 0 to 96 bytes against a
   fixed valid header template, plus a second pass over lengths 0-31 built
   from non-template bytes, checking the parser rejects everything under 32
   bytes and that its outcome for a fixed 32-byte prefix never depends on
   how many trailing bytes follow it.
2. **Single-byte field exhaustive sweep**: all 256 possible values for each
   single-byte field the header decodes (version, last-update
   year/month/day, table flags, code-page mark), confirming every derived
   accessor (`version_description`, `has_memo_file`, `dbf_code_page_from_mark`,
   etc.) stays safe to call for the full byte range of the fields it reads.
3. **Multi-byte boundary-value sweep**: classic boundary-value analysis
   (minimum, minimum+1, values just below/at/above the 32-byte
   `header_length` threshold, and the 16-/32-bit range edges) for
   `header_length`, `record_length`, and `record_count`.
4. **Deterministic synthetic-random sweep**: 5,000 cases from a
   `std::mt19937` generator seeded with a fixed constant (the algorithm is
   fully specified by the C++ standard, so the exact same sequence of cases
   runs on every platform and every invocation), covering 0-512-byte
   arbitrary buffers.
5. **File-based inputs**: `parse_dbf_header_from_file` against a nonexistent
   path, an empty file, a one-byte-short file, a valid synthetic file, and a
   4096-byte deterministic garbage file, all under a dedicated temporary
   directory the test creates and removes.

Every sweep is a fixed, compile-time-bounded loop -- there is no
open-ended, coverage-guided, or corpus-mutating search, and no unbounded
loop exists inside the code under test for any single call to hang on. The
test also asserts its own total wall-clock time stays under a fixed 20-second
budget as an explicit, auditable ceiling on top of that structural bound.
Total case count and duration are reported in the JSON summary described
below (6,752 cases in well under a second in local testing).

Resource limits: input sizes are capped (at most 512 bytes for the
synthetic-random sweep, a few kilobytes for the largest file-based case);
iteration counts are fixed constants, not runtime-configurable; the test is
single-threaded (a concurrency limit of one).

## Sanitizer variant

When the local C++ compiler supports it (detected via
`CMAKE_REQUIRED_LINK_OPTIONS`, since sanitizer support requires the flag at
both compile and link time -- `check_cxx_compiler_flag` alone only threads a
flag through the compile step of its probe and cannot detect this), an
additional `test_dbf_header_robustness_sanitized` CTest target compiles
`src/vfp/dbf_header.cpp` as a fresh translation unit instrumented with
AddressSanitizer and UndefinedBehaviorSanitizer (`-fsanitize=address,undefined
-fno-sanitize-recover=all`) and links it against the same test source. This
is purely additive: the normal `cf_vfp_assets` library and the default
`test_dbf_header_robustness` target are compiled exactly as before, so
normal and release builds are unchanged whether or not the sanitized variant
exists. If the compiler does not support the flag combination, the sanitized
target and test are not created at all.

## Machine-readable result summary

Each run of `test_dbf_header_robustness` (and, separately, of the sanitized
variant) writes `dbf_header_robustness_report.json` into its CTest working
directory:

```json
{
  "schema_version": 1,
  "kind": "copperfin-dbf-header-robustness-result",
  "target": "copperfin::vfp::parse_dbf_header",
  "defense_in_depth_only": true,
  "release_gate": false,
  "exhaustive_coverage_claimed": false,
  "deterministic": true,
  "total_cases": 6752,
  "failures": 0,
  "elapsed_milliseconds": 29,
  "wall_clock_budget_seconds": 20,
  "status": "passed"
}
```

## Result

Local Linux runs of both the normal and sanitized variants pass with zero
failures and no AddressSanitizer or UndefinedBehaviorSanitizer findings
across all 6,752 bounded, deterministic cases. That is evidence the header
parser handles this test's boundary and arbitrary-byte cases safely; it is
not a claim that the parser (or any other DBF-family parser in this
codebase) is exhaustively verified or free of every possible defect.
