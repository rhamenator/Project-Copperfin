# CAST INT16/SHORT Numeric/exact conversion

RQ-CF-PRG-CAST-INT16-NUMERIC-001, admitted #5611/#6776, from origin/main
3d78c5a3a2ac597b5369ada811806d554552eb26. This requirement/architecture/
verification mapping is recorded2026-10-10T15:17Z BEFORE helper/tests/configuration
or dispatcher migration. Existing Copperfin output and sibling tests are
verification only, never requirement sources.

## Independent evidence and derived extension contract

Two independent installed Wine VFP9 SP2 09.00.0000.7423 observations match
byte-for-byte:89lines/87calls/DONE. Each has29 ordinary INTEGER controls and
58 unsupported INT16/SHORT target errors11, including0. Ordinary INTEGER
truncates1.9/-1.9 to1/-1,32767.9 to32767 and-32768.9 to-32768; it admits32768
and-32769 as INTEGER, NOT signed16. Unsupported syntax is NOT a native16
range/wrap policy. Full scripts/results/empty stdout/stderr are retained.
Both bounded55s invocations exited0, fresh distinct outputs; no incomplete
native observation or timeout-optimum/root-cause claim.
Shipped Microsoft CAST help55c599e9-18d1-4ac6-b311-e6677dc3b0b7.htm,
SHA8bf6b2a5a06b705090f6597d63e14b359e91146ebe255120923ac0a11404a21d,
lists I/INT/INTEGER and potentially lossy Numeric conversion, not INT16/SHORT.
No decompilation/binary inspection/VM start; backup-reset completion unconfirmed.

Parent RQ-CF-PRG-NUMERIC-BEHAVIOR-001/#5611/#6776 requires defined,
operation-specific checked conversion. Direct owner extension intent2026-10-06
supports useful signed16 capability even though native VFP lacks its syntax.
Independent signed16 mathematics defines -32768..32767. Native controls
support truncation only; the following range/exact/NaN boundaries are delegated
modern-extension requirements, not emulated native signed16 quirks.

Both COPPERFIN and VFP9 modes share the SAME extension policy:

- Numeric: finite input, truncate toward zero, admit truncated result in
  [-32768,32768), return exact int64 holding a signed16 value. Both1.9/-1.9
  truncate;32767.9=>32767 and-32768.9=>-32768;32768/-32769 reject.
  Do not wrap low16, clamp or invent a native quirk in VFP9 mode.
- Exact int64: compare directly against -32768..32767 without Double;
  exact uint64: admit through32767, then safely return int64. Wide exact
  values including2^53+1/INT64_MIN/MAX/UINT64_MAX reject rather than alias.
- Invalid Numeric/exact domain: localized catchable11 BEFORE assignment.
  NaN/infinities/huge values reject before any floating-to-integral cast.
- Non-Numeric/global NULL/type/Currency/arity paths are untouched preservation
  controls, not newly recovered safety or native compatibility requirements.

Independent binary64 math: nextafter(32768,0)=32767.999999999996 truncates32767;
32768/nextafter(32768,+infinity)=32768.00000000001 reject.
nextafter(-32769,0)=-32768.99999999999 truncates-32768;
-32769/nextafter(-32769,-infinity)=-32769.00000000001 reject.
Exact nested sums construct2^53+1/INT64_MAX/UINT64_MAX from representable
components; rounded Numeric literals are NOT exact64 inputs.

Architecture: distinct checked_cast_int16_numeric_argument for number/int64/
uint64 only, UNUSED by CAST through BOTH complete original GNU/Clang runs.
Then ONLY INT16/SHORT Numeric-kind dispatcher gate migrates, preserving
non-Numeric fallback/NULL, all other CAST/shared conversions/arithmetic/parser/
settings. No new setting; both mode/session shards prove cursor/context/reset.

## Verification and assurance plan

VR-5611-CAST-INT16-NUMERIC-001: independent literal direct helper fractions/
endpoints/nextafter/huge/nonfinite/NaN/exact kinds and non-Numeric adapter guards.
VR-5611-CAST-INT16-NUMERIC-002: both aliases, four fresh mode/session shards,
exact kind/value/error11, sentinel retention, continuation/INTEGER control,
cursor/context/reset, four literal catalog messages. Public NaN syntax is not
invented. Unchanged signed32/signed64/unsigned64/BYTE/exact-expression and
seven shared-script bit neighbors; broader Numeric/NULL/typed-NULL/string-math/
localization/seven repository contracts.
Freeze all independent inputs/counts/helper/header/tests/CMake/isolation BEFORE
configure. Retain BOTH complete GNU/Clang original full raw logs/counters/
hashes/cmp BEFORE migration. Original Clang recovery/halt0 finishes cases,
not fixed acceptance. Same fixed flags halt1/leaksdisabled. Input/assertion
repair requires explicit refreeze and BOTH complete original repeats.
Serial nice10/ionice2:7/-j1/private RAM scratch, no local build/test/Wine overlap
or suspended owned test across heartbeats.

DQ-5611-CAST-INT16-NUMERIC-001 delta: replace unbounded Numeric/exact INT16
conversion with signed16 admission/atomic11, preserve fractional truncation
and exact integer kind/precision; both modes agree. Bounded misuse medium:
mistaking aliases for native VFP syntax, expecting low16 wrapping in VFP9,
or treating rounded Numeric as exact64. HZ-runtime-crash-01/
HZ-data-corruption-01/HZ-doc-command-01 linked, registered severities remain
active/unchanged; no new allocation/process/persistence boundary or KBX.
DV-5611-CAST-INT16-NUMERIC-001 development self-review/automated walkthrough
checks both fractional endpoints, out-of-range/exact rejection, no assignment,
continued controls/context/reset and catalogs. Completed evidence is retained
in baseline-audit.md. Not independent-human/high-hazard
qualification. Rollback bounded code/docs together, retain raw/native tests,
correct guidance/notify owner if published guidance is wrong.
Signed/DCO postpush, Claude FIRST/authorized verified Codex fallback, actual
clean exact-head review/all required green/resolved conversations still gate
integration. No family/type/Currency/arity/leak/platform/release/parent closure.

Current local evidence: both complete original GNU/Clang33-test comparisons
were retained before Numeric-only migration (29/33 each,920selected/zero other
assertions; four original Clang float-cast-overflow reports). Unchanged frozen
inputs then pass fixed GNU33/33 and Clang33/33 with no fixed sanitizer reports.
Full raw logs, counts, hashes, preliminary GNU foreign-overlap disclosure and
separate serial comparison are in [baseline-audit.md](baseline-audit.md).
Broader GNU12/12 (one invocation) and Clang4/4 PASS/no sanitizer reports;
bounded medium DQ/DV development walkthrough complete. Signed-head integration
is still pending; local success is not a full-platform or parent-workstream
acceptance claim.
