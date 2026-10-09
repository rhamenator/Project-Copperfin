# BITSET Numeric/exact value and position conversion

RQ-CF-PRG-BITSET-NUMERIC-001 mapped before helper/test creation or migration.
Bounded owner-admitted #5611/#6776/#6871 slice at main
8b2a28ac4a07b5bb85c45f2fbc0d5af8279e7b06; no independent defect-hunt lane.

## Independent recovery

Installed Wine VFP9 SP2 09.00.0000.7423 and shipped help topic
0279f322-82f1-4687-82ac-a0d772f25e72.htm independently establish Numeric
BITSET(value, position), integer conversion before setting a bit, positions0..31
with0 rightmost. Binary optional start/count is a different overload, not a
Numeric third-argument requirement. No binary inspection/decompilation used.
Initial plus three serial fresh25s harness runs retained here, nice10/ionice2:7;
no VM started. Native value observations at positions0 AND1 distinguish the
conversion even when one set bit hides an original bit. Position observations
use0 AND-1, so absorbing value-1 cannot hide an invalid position; all32 valid
bits and mixed controls are retained independently. Native fractions truncate
toward zero; signed low32 aliases, huge finite and both infinities convert0,
then position0..31 is checked with error11. Neither exact64 nor NaN native
semantics are claimed.

## Derived requirement and boundary

Parent RQ-CF-PRG-NUMERIC-BEHAVIOR-001 and owner #6776 require COPPERFIN finite
truncated/exact signed32 value and position0..31, otherwise localized catchable11
before result assignment. Explicit VFP9 preserves independently observed low32/
huge/infinity0 conversion before range validation. Exact64 low bits without a
Double round trip and NaN rejection derive from parent safety intent, not native
extension evidence. Validate both arguments, including a late invalid position
after-1; uint32 shift only after0..31 admission and OR, defined signed32 result
in int64. No argument-driven allocations/loops, persistence or cursor changes.

Distinct checked value and position helpers remained unused by BITSET through
BOTH unchanged original GNU/Clang baselines. Only BITSET dispatcher migrated
after those completed; shared bitwise_value/bit_position, siblings, existing
type/Currency/binary/arity/global NULL policy remain unchanged. Checked safe
other-type controls preserve existing behavior, NOT native type parity.
Unsafe non-Numeric conversion rejects rather than wraps; parent-derived safety.

## Frozen verification plan

VR-5611-BITSET-NUMERIC-001/002 cover independent literal value/position/all32/
mixed native rows, both modes/sessions1/2, exact64/nextafter/nonfinite controls,
assignment preservation/error11/continuation and selected-session/cursor/mode/
reset. Direct helpers cover NaN inaccessible to this native expression fixture.
Two existing expression neighbors reused unchanged. Freeze helper/header/tests/
CMake/isolation hashes BEFORE configuration; retain complete GNU and Clang
ASan/UBSan/float-cast-overflow originals before migration. Any frozen assertion
repair/removal/weakening requires explicit refreeze and BOTH original repeats.
Pending original/fixed/broader/contracts/review/hosted evidence is not acceptance.

Native source SHA-256
9e52514f506d85acf1f05b4977ffe1431fb3d7daaef7614c80624c91adfdd531;
all five retained outputs SHA-256
da4f79c50e984a5ea7ba96979c902157970c1725357db2b978fd3deee10cb837.
Frozen plan206 direct/792 public cases; source hashes in baseline-audit.md.

## Hazard, documentation and rollback boundary

HZ-runtime-crash-01/HZ-data-corruption-01/HZ-doc-command-01 remain active at
registered severities. This bounded command-use guidance has medium misuse:
VFP9 aliases can conceal an invalid untrusted value/position; default callers
must catch11. No safety-critical operating procedure or hazard closure claimed.
DQ-5611-BITSET-NUMERIC-001 procedural delta: Numeric half-away rounding becomes
truncation; COPPERFIN rejects wide/nonfinite values and invalid truncated
positions before assignment, VFP9 selects observed low32/huge/infinity0 before
position validation. Even value-1 must not skip validation of a later position.
No KBX exception is applied.

DV-5611-BITSET-NUMERIC-001 requires completed development self-review and
automated walkthrough of7.9,1.9 ->7,0,31.9 ->-2147483648; invalid late32 after-1
must retain nResult12345, continuation/control7 and selected session/cursor/mode,
then reset1/COPPERFIN/closed cursor. Exact64/NaN/nextafter and both modes must
pass. Review/walkthrough evidence remains pending at freeze, not acceptance or
independent-human/high-hazard qualification. Rollback: revert only this slice,
correct command guidance and notify affected integrators if incorrect; retain
native/original/fixed logs. No production persistence changes or VM testing.

## Completed bounded local verification and development assurance

Both complete unchanged originals retained BEFORE migration: GNU15.15s and
Clang30.00s, actual206direct/792public,540 byte-identical selected failures
(246error/266result/28kind;398COPPERFIN/142VFP9), zero unrelated or sanitizer
diagnostics. First disk-backed GNU attempt interrupted130 after an observed
journal wait at2m16s; its incomplete198-line log is retained separately, never
claimed as all-scenario evidence. Same binary/frozen inputs repeated in private
RAM-backed TMPDIR, with no workflow/test-definition timeout or input change.
Timings are observations, not disk/persistence/timeout-optimum qualification.

BITSET-only migration then passed GNU focused2/2 in14.88s and broader17/17
in503.65s (older Numeric/NULL/bit siblings/Collection/localization/seven contracts).
Safety contract kept1200s cap. Fixed Clang ASan/UBSan/float-cast-overflow9/9
PASS167.39s, no diagnostics/leaks disabled. All actual counters/full raw logs/
source/output SHA-256 and chronology retained in baseline-audit.md. Six frozen
inputs/shared platform unchanged; only selected dispatch migrated.

Implementation-agent delegated development self-review completed2026-10-09:
allowed native literals and parent-derived safety compared against checked
value/position helpers and BITSET-only dispatch. Admission precedes narrowing/
shift; exact64 avoids Double round trips, NaN rejects, native infinity0 exists
only in explicit VFP9; even all-ones value validates a later position. Defined
uint32 OR/signed32 arithmetic preserves int64 kind. No argument-sized work or
persistence changes, shared helpers/siblings untouched. Safe coercion controls
are preservation, not native type/Currency/binary/arity parity.

DV-5611-BITSET-NUMERIC-001 automated GNU/Clang walkthrough passed:7.9,1.9 ->7;
0,31.9 ->-2147483648; invalid late32 after-1 yields11 with nResult12345 intact,
continuation/control7 and cursor/session/mode retained, reset1/COPPERFIN/closed
cursor. Direct NaN/nextafter/exact64 controls and both modes/selected sessions
pass. DQ/DV bounded medium-misuse development self-review/walkthrough/rollback
is complete, not independent-human/high-hazard sign-off or closure of active
registered hazards. Signed postpush exact-head verification, actual clean
external review/required hosted checks/resolved conversations remain integration
gates. No type/binary/arity/full-native/platform/leak/release/family/parent
acceptance; BITCLEAR/BITTEST still require their own bounded evidence.
