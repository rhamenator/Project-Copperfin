# CAST signed64 Numeric/exact conversion

RQ-CF-PRG-CAST-INT64-NUMERIC-001, bounded admitted #5611/#6776 slice from
origin/main8311e7af8c9a0c676e532a5a43c3934a785dbd0e. Requirement/architecture/
verification mapping precedes new helpers, tests and production migration.

## Independent native boundary and extension derivation

Installed Wine VFP9 SP2 09.00.0000.7423 and shipped Microsoft CAST help
55c599e9-18d1-4ac6-b311-e6677dc3b0b7.htm document ordinary INTEGER (I/INT),
not INT64/LONGLONG/BIGINT. Local shipped help SHA-256:
8bf6b2a5a06b705090f6597d63e14b359e91146ebe255120923ac0a11404a21d.
No binary inspection, disassembly or decompilation was used.

Three fresh serial bounded25s Wine runs on2026-10-10 retain byte-identical
32-line/30-call output:15 ordinary INTEGER/INT/I controls pass, including
1.9=>1,-1.9=>-1 and both signed32 endpoints. All15 signed64 alias observations
raise11, including zero. Native error11 here is an unsupported target-type
boundary, NOT a numeric range contract for a native signed64 type. No VM
started; each run used nice10/ionice2:7 and had empty stderr.

Source probe.prg SHA-256:
09dba0553343c294b37fc90bc5f96b4387a7416f55e1916a0e7d96c8bf2ec032.
Each native-1.out/native-2.out/native-3.out SHA-256:
2d2e0d79855859167db6cd73f990f42bb42b364651d3c55e3c0be8cb969e4f71.
Scratch sources/results were compared byte-for-byte with retained fixture files.

Owner extension intent2026-10-06 retains meaningful modern capabilities even
when VFP lacks syntax. Parent RQ-CF-PRG-NUMERIC-BEHAVIOR-001/#5611/#6776
requires defined operation-specific conversion and exact64 safety. Independent
integer mathematics supplies the signed64 interval[-9223372036854775808,
9223372036854775807]. A binary64 Numeric just below2^63 is9223372036854774784;
2^63 is outside, while -2^63 is exactly representable and inside. The next
Numeric below -2^63 is -9223372036854777856 and outside. Decimal INT64_MAX
as a Numeric rounds to2^63; that Numeric must reject, while an exact int64
INT64_MAX must succeed without a Double round trip. Ordinary fraction controls
support truncation, not rounding; they do not establish native64 semantics.

Derived contract, identical in COPPERFIN and VFP9 modes:

- Numeric: finite, truncate toward zero within[-2^63,2^63), then signed64 result.
- Exact int64: identity, including min/max and low bits above2^53.
- Exact uint64: admit through INT64_MAX, reject above it before signed conversion.
- Invalid Numeric/exact64 domain: localized catchable11 before assignment. This error is the
  parent operation-specific conversion policy, not exact arithmetic's error39
  and not invented native64 overflow behavior. No low32 alias, wrap or clamp.
- Ordinary other coercions and existing global NULL handling are preservation
  controls only; no new native type/Currency/binary/arity policy is claimed.

Only the signed64 dispatcher branch and its distinct checked_cast_int64_argument
adapter are selected. Reuse the proven checked_declared_int64_argument without
changing it or its other consumers. Unsigned/INT32/INT16/BYTE/other CAST branches,
parser, shared arithmetic and session settings are outside this slice.

## Verification plan and assurance boundary

VR-5611-CAST-INT64-NUMERIC-001: independent literal helper checks for fractions,
signed64 endpoints/nextafter/huge/nonfinite and exact signed/unsigned precision.
VR-5611-CAST-INT64-NUMERIC-002: fresh public CAST expressions for all3aliases,
both Numeric modes and sessions1/2; exact result kind/value, catchable11,
unchanged sentinel assignment, subsequent control, cursor/context/reset and
four-catalog localized error evidence. Existing exact-expression/runtime-surface/
Numeric/NULL tests remain neighbors, not requirement sources.

Freeze helpers/header/tests/CMake/isolation before configuration. Keep the new
helper unused by CAST through BOTH complete original GNU and Clang baselines.
Clang original instrumentation must report unsafe conversions while allowing
the entire frozen baseline to complete (float-cast-overflow recovery enabled,
UBSAN halt_on_error=0); a completed diagnostic-bearing run is negative evidence,
NOT sanitizer acceptance. Fixed verification uses halt_on_error=1. Retain full
raw logs, counts, hashes, chronology and any incomplete attempts. Any assertion
repair/removal/weakening requires explicit refreeze and BOTH original repeats.
Helper/header/tests/CMake/isolation are now frozen at00:55Z (56direct,
4x141public); the adapter remained unused by CAST through both originals.
See baseline-audit.md for frozen hashes and execution settings.

The first GNU attempt aborted in a wrongly specified nonnumeric Character
preservation control. It is retained in full, not counted as a complete baseline.
Explicit01:17Z refreeze keeps every input and all Numeric/exact64 expectations;
Character 'x' now checks its unchanged coercion exception/public error1 and
assignment retention, rather than an invented zero. Complete refrozen GNU:
56direct/4x141public and both neighbors complete,672 selected assertions and
zero other failures,7.25s. Complete refrozen Clang original: same counts and672
selected assertions/zero other,13.57s, with four actual UBSan reports at the
original signed64 dispatcher conversion. Only after retaining both complete
originals was that branch migrated; helpers/frozen tests remain unchanged.
Full raw logs/hashes/chronology are in baseline-audit.md.

Fixed GNU focused7/7 PASS7.54s; fixed Clang focused7/7 PASS9.55s with ASan,
UBSan and float-cast-overflow instrumentation, halt_on_error=1, no diagnostics
and leaks disabled. Actual56direct/4x141public counts complete in both runs.
Full raw102-line logs retained as fixed-gcc.log and fixed-clang.log; SHA-256:
eaf509949c789b7b1ae432eda262e7261a719eb972c186cb17cd992b56648ea4 (GNU),
1be26a860f54a6b281fd6596a9986172e2d848c8f8cb60a12baf627f4e414ac5 (Clang).
Broader GNU11/11 PASS421.84s plus separately selected portable CLR contract
1/1PASS0.04s (the initial regex missed its actual name); all seven selected
contracts and Numeric/NULL/typed-NULL/string-math/localization pass. Broader
Clang4/4PASS106.67s for the four unchanged runtime neighbors, same sanitizer
flags/halt1/no diagnostics/leaks disabled. Full raw162/35/69-line logs and
SHA-256 hashes are retained in baseline-audit.md. The locale-install contract's
expected missing-catalog negative subprocess diagnostic is retained, not an
unexpected CTest failure. Integration gates remain pending.
RAM-backed fixture testing does not qualify disk persistence,
timeout optima, leaks, other CAST target types or full-platform behavior.

HZ-runtime-crash-01/HZ-data-corruption-01/HZ-doc-command-01 stay active at their
registered severities. This bounded change has medium documentation misuse:
callers may confuse a rounded Numeric with an exact64 identifier, or assume
VFP9 mode makes a nonexistent native64 wrap behavior available. No new critical
operator procedure, argument-sized allocation, external process or persistence
mutation is introduced. No KBX exception applies.

DQ-5611-CAST-INT64-NUMERIC-001 procedural delta: undefined/wrapped conversions
become error11; exact64 identity preserves low digits; explicit VFP9 mode does
not change this extension contract. DV-5611-CAST-INT64-NUMERIC-001 requires
completed delegated development self-review and automated GNU/Clang walkthrough:
CAST(1.9 AS INT64)=>1; re-CAST exact9007199254740993 keeps its low bit;
Numeric2^63 rejects11 with sentinel unchanged and execution continuing; exact
INT64_MAX succeeds; both modes/sessions/context/reset/localization remain valid.
The bounded medium-misuse delegated development self-review and automated
walkthrough are complete: see the explicit code/expected-outcome/results record
in baseline-audit.md. This is not independent human/high-hazard qualification
or hazard closure.

Rollback: revert only this slice, preserve native/original/fixed evidence and
boundary tests; correct guidance and notify affected integrators if published
guidance is wrong. Hosted exact-head checks/clean Claude or authorized Codex
review/resolved conversations remain integration gates. Full native signed64,
other types/unsigned CAST, platform/leak/release/family/parent acceptance separate.
