# CAST BYTE/UINT8 Numeric/exact conversion

RQ-CF-PRG-CAST-BYTE-NUMERIC-001, admitted #5611/#6776, from origin/main
c1a6eef528c1d621196a6c7e232cf8e399174ea0. This requirement/architecture/
verification mapping precedes helper, test, configuration and migration edits.
Existing Copperfin masks, coercions and sibling tests are verification only,
never requirement sources.

## Independent evidence and derived extension contract

Installed Wine VFP9 SP2 09.00.0000.7423 independently executes 25 values across
INTEGER/BYTE/UINT8. The complete first observation has 77 lines/75 calls/DONE:
ordinary INTEGER controls truncate 0.9/-0.9 to0, 1.9 to1, -1.9 to-1 and
255.9 to255. BYTE and UINT8 each reject all25 values with11, including zero.
This is unsupported target syntax, NOT a native unsigned8 range/wrap policy.
Shipped Microsoft CAST help 55c599e9-18d1-4ac6-b311-e6677dc3b0b7.htm, SHA-256
8bf6b2a5a06b705090f6597d63e14b359e91146ebe255120923ac0a11404a21d,
names I/INT/INTEGER and potentially lossy Numeric conversion, not BYTE/UINT8.
No binary inspection, decompilation or VM start; backup-reset completion is
unconfirmed. Native repeats and any incomplete attempts remain separate in
baseline-audit.md, not evidence of native exact64 or IEEE input bits.

Parent RQ-CF-PRG-NUMERIC-BEHAVIOR-001/#5611/#6776 requires defined checked
operation-specific conversion. Direct owner extension intent2026-10-06 permits
meaningful unsigned8 capability without inventing native syntax or quirks.
Independent integer mathematics defines unsigned8 as0..255. Native INTEGER
supports fractional truncation only; representable unsigned8 admission below
is a delegated modern-extension requirement, not native BYTE behavior.

Both COPPERFIN and VFP9 modes have the SAME extension policy:

- Numeric: require finite input, truncate toward zero, admit truncated values
  in[0,256), return exact uint64 holding0..255. -0.9 becomes0; -1 rejects;
  255.9 becomes255; 256 rejects. Do not wrap, mask, clamp or invent a native
  unsigned8 quirk in VFP9 mode.
- Exact signed64: admit0..255 without a Double round trip; reject negatives
  and256 or above, including INT64_MIN/MAX. Exact unsigned64: admit0..255;
  reject larger values, including2^53+1 and UINT64_MAX. Low-bit residues are
  NOT range admission; e.g. exact2^53+1 must reject, not become1.
- Invalid Numeric/exact domain: localized catchable11 before assignment.
  NaN/infinities/huge values reject before any floating-to-integral cast.
  Error11 is the parent conversion policy, not arithmetic overflow39.
- Non-Numeric coercion/global NULL/type/Currency/arity behavior stays unchanged
  and is only preservation evidence, not a newly recovered safety contract.

Independent binary64 math: nextafter(256,0)=255.99999999999997 truncates255;
256 and nextafter(256,+infinity)=256.00000000000006 truncate256 and reject.
nextafter(-1,0)=-0.9999999999999999 truncates0;
nextafter(-1,-infinity)=-1.0000000000000002 truncates-1 and rejects.
These literal expectations do not come from Copperfin output. Exact nested
integer sums construct2^53+1, INT64_MAX and UINT64_MAX from independently
known representable parts; source Numeric spellings are not exact64 inputs.

Architecture: distinct checked_cast_byte_numeric_argument for number/int64/
uint64 only, UNUSED by CAST through BOTH complete originals. Then only the
BYTE/UINT8 Numeric dispatcher gate migrates; original non-Numeric path,
INT16/SHORT/other CAST/shared conversions/arithmetic/parser/settings untouched.
No new session setting; public cases must prove both selected modes and
sessions1/2 retain cursor/context/reset. Neighbor CAST tests are not oracles.

## Verification and assurance plan

VR-5611-CAST-BYTE-NUMERIC-001: independent literal direct helper cases for
fractions/endpoints/nextafter/huge/nonfinite/NaN/exact kinds and fail-closed
non-Numeric adapter boundary. VR-5611-CAST-BYTE-NUMERIC-002: two aliases,
fresh public cases in both modes/sessions1/2; exact kind/value/error11,
sentinel retention, continuation/control, cursor/context/reset, four catalogs.
Unchanged signed32/signed64/unsigned64 CAST and exact-expression neighbors;
broader Numeric/NULL/typed-NULL/string-math/localization/seven contracts.

Freeze independent inputs/counts/helper/header/tests/CMake/isolation BEFORE
configuration. Keep helper unused through BOTH complete GNU and Clang originals;
retain full raw logs/counters/hashes BEFORE Numeric-only migration. Original
Clang ASan/UBSan/float-cast-overflow recovery/halt0 finishes all cases, NOT
clean acceptance. Fixed same flags use halt1/leaks disabled. Assertion/input
repair requires explicit refreeze and BOTH original repeats. Serial nice10/
ionice2:7/-j1/private RAM fixtures; no local build/test/Wine overlap or owned
paused test across heartbeats. No performance/disk/timeout-optimum qualification.

2026-10-10T10:25Z freeze:140direct and4x126fresh public cases. Exact count
derivation and helper/header/tests/CMake/isolation hashes in baseline-audit.md;
helper remained UNUSED by CAST through both complete originals. GNU original17/21PASS36.80s:
all140/four126new and unchanged-neighbor counters,872selected/zeroother
assertions. Full1431-line raw/cmp/hash retained before Clang configuration.
Clang original17/21PASS50.56s, configure/build0/CTest8: same complete counters
and872selected/zeroother assertions, plus four actual recoverable float-cast-
overflow reports at the original BYTE conversion for -1. Full1526-line raw
copied/cmp/hash retained BEFORE migration; all five freeze hashes unchanged.
Both negative originals are comparison evidence, NOT fixed acceptance.
Only Numeric BYTE/UINT8 dispatch migrated after both logs were retained.
Fixed GNU21/21PASS36.63s and Clang21/21PASS46.05s, build/CTest0 each,
all frozen new/neighbor counters and zero assertions; fixed Clang halt1 has
no sanitizer report, leaks disabled. Full274/275-line raw logs retained/cmp/
hash in baseline-audit.md. Existing unrelated unused-function warnings are
retained, NOT warning-free qualification. No frozen input/helper/test change.
Broader GNU12/12PASS417.07s (four runtime/localization/seven contracts) and
Clang sanitizer4/4PASS114.85s; full230/80-line raws/cmp/hashes retained, no
sanitizer reports. Hosted/review integration acceptance remains pending.

DQ-5611-CAST-BYTE-NUMERIC-001 procedural delta: reject Numeric/exact unsigned8
overflow instead of undefined conversion or low-bit masking; preserve exact
kinds and negative-fraction truncation; both modes agree for this extension.
Bounded documentation misuse medium: mistaking BYTE for native VFP syntax,
expecting VFP9 mode to wrap, or treating a rounded Numeric as an exact integer.
HZ-runtime-crash-01/HZ-data-corruption-01/HZ-doc-command-01 remain active at
registered severities; no new critical operator procedure/allocation/process/
persistence boundary or KBX. DV-5611-CAST-BYTE-NUMERIC-001 bounded
delegated development self-review plus automated GNU/Clang walkthrough completed
in baseline-audit.md:
-0.9=>0, 255.9=>255, -1/256/huge reject11 without assignment, exact wide values
reject rather than alias, continued controls/context/reset/catalogs unchanged.
Not independent-human/high-hazard qualification. Rollback only bounded code/
docs together, retain native/raw tests, correct guidance and notify owner if
published guidance is wrong. Review/hosted gates remain pending.
Signed postpush proof, actual clean exact-head Claude-first/Codex-fallback,
all required checks and resolved conversations gate integration. No full native
type/Currency/arity/leak/platform/release/family/parent acceptance.
