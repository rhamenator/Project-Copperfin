# DBF Header Parser Fuzzing

## Status

This is **optional defense-in-depth testing**, not a v1 or RC release gate.
It does not block any build, workflow, or evaluation candidate, and it does
not claim exhaustive format or security coverage.

## Purpose

`tests/test_dbf_header_robustness.cpp`
(docs/36-dbf-header-parser-robustness-testing.md) gives
`copperfin::vfp::parse_dbf_header` bounded, deterministic coverage: a fixed
set of hand-chosen boundary values and a fixed, seeded pseudo-random sweep.
That is complementary to, not a substitute for, coverage-guided fuzzing --
libFuzzer's mutation engine explores byte combinations no one wrote down in
advance, using code coverage feedback to steer toward new paths through the
parser. `tests/fuzz_dbf_header.cpp` adds that open-ended search on top of the
same target.

This slice was previously attempted as a libFuzzer-based work item on
`agent/v1-dbf-header-fuzzing`; that branch was never pushed and no longer
exists. This is a fresh implementation, not a recovery of that earlier work.

## Scope

Identical to docs/36-dbf-header-parser-robustness-testing.md: in scope is
`parse_dbf_header` and the `DbfHeader` accessor methods reachable from a
successful parse (`format_family`, `has_database_container`,
`has_production_index`, `has_structural_cdx`, `has_memo_file`,
`version_description`, `last_update_iso8601`), plus
`dbf_code_page_from_mark` and `dbf_format_family_name`. Out of scope,
deliberately: the field-descriptor array and record data that follow the
header, CDX/IDX/NDX/MDX index parsing, and memo file parsing -- the same
exclusions as the deterministic robustness test, for the same reason (these
are separate parsers with their own boundary conditions). This harness
targets only Copperfin's own code, downloads no external corpus, and makes
no network access; `parse_dbf_header_from_file` (the file-path-based entry
point) is not exercised here since libFuzzer feeds `parse_dbf_header`
in-memory buffers directly.

## What the harness does

`tests/fuzz_dbf_header.cpp` is a `LLVMFuzzerTestOneInput` entry point: it
calls `parse_dbf_header` on the raw fuzzer-provided bytes and, on a
successful parse, calls every derived accessor listed above so libFuzzer's
sanitizer instrumentation can catch a defect reachable only through one of
them, not just the initial parse call.

Unlike the deterministic robustness test, this is intentionally open-ended:
there is no fixed case count, and which inputs are tried is decided by
libFuzzer's coverage-guided mutation, not a compile-time constant. Two
things keep a CI-invoked run bounded and reproducible instead of an
unbounded background search:

- **Fixed wall-clock budget.** The CTest-invoked run passes
  `-max_total_time=20`, so a single invocation cannot run indefinitely.
- **Fixed seed.** `-seed=1` makes libFuzzer's mutation sequence reproducible
  for a given libFuzzer version and starting corpus -- not a guarantee of
  byte-identical results across different libFuzzer/LLVM versions or
  platforms, but enough that a local re-run against the same toolchain
  reproduces the same path through the search.

### Seed corpus

`tests/fuzz/corpus/dbf_header/` holds eight small, synthetic, checked-in seed
files, deliberately mirroring the same fixed points the deterministic
robustness test already covers by construction (so the fuzzer starts from
known-interesting bytes rather than an empty corpus): a full 96-byte valid
Visual FoxPro header, that header's minimal 32-byte prefix, a 31-byte input
one byte short of the minimum, an empty file, a header with `header_length`
exactly at its 32-byte threshold, all-zero and all-`0xFF` 32-byte buffers,
and a classic dBASE III header (version `0x03`, no memo/CDX flags). None of
these are downloaded or derived from any third-party file.

**This directory is seed-only and must never be passed directly as
libFuzzer's live corpus argument.** libFuzzer treats its corpus argument as
read-write and adds every newly-covering input it finds; pointing it at this
source-tree directory would let a local or CI run silently commit generated
files into version control. `run_dbf_header_fuzz_smoke_check.cmake` resets a
build-tree corpus from these checked-in seeds immediately before every test
run and only ever points libFuzzer at that copy -- not just once at configure
time -- so a second `ctest` invocation in the same build tree runs from the
fixed seeds again rather than a corpus a prior run already grew.

## Running the optional targets

The target is absent from default and release builds, and only builds when
the compiler supports libFuzzer plus ASan/UBSan together (not available
under MSVC). Enable and run it explicitly from an out-of-tree build:

```sh
cmake -S . -B build-fuzz \
  -DCOPPERFIN_BUILD_TESTS=ON \
  -DCOPPERFIN_BUILD_FUZZ_TESTS=ON
cmake --build build-fuzz --target fuzz_dbf_header
ctest --test-dir build-fuzz -R '^test_dbf_header_fuzz_smoke$' --output-on-failure
```

`test_dbf_header_fuzz_smoke` is the bounded, fixed-seed CI-facing run
described above. For a longer, genuinely exploratory local session (not
CTest-invoked, and not fixed-seed), run the built binary directly against
its own corpus copy, or a separate scratch directory, with a larger time
budget, e.g.:

```sh
mkdir -p /tmp/dbf-header-fuzz-corpus
cp tests/fuzz/corpus/dbf_header/* /tmp/dbf-header-fuzz-corpus/
build-fuzz/tests/fuzz_dbf_header -max_total_time=600 /tmp/dbf-header-fuzz-corpus
```

Never point a long-running or unseeded session at
`tests/fuzz/corpus/dbf_header/` itself, for the reason above.

## Result

A local Clang 21.1.8 (Linux) build of `fuzz_dbf_header`
(`-fsanitize=fuzzer,address,undefined -fno-sanitize-recover=all`) ran the
bounded `test_dbf_header_fuzz_smoke` invocation (`-max_total_time=20
-seed=1 -runs=200000` against the eight-file seed corpus) to completion: it
stopped at its time budget after 63,682 executions (exec/s ~3,000-3,300,
corpus grown to 41 entries by coverage-guided minimization) with zero
crashes, sanitizer aborts, or timeouts. That is evidence the header parser
and its derived accessors handled that search's inputs safely within the
allotted budget; it is not a claim of exhaustive coverage, and it does not
supersede the deterministic robustness test's own evidence for the fixed
cases that test checks explicitly.
