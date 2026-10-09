# BITCLEAR Numeric/exact value and position

RQ-CF-PRG-BITCLEAR-NUMERIC-001 mapped BEFORE helper/test creation or migration.
Bounded owner-admitted #5611/#6776/#6871 slice at origin/main
1d23bd7a9c63238a6966e9503c32a94a5e89b639. No independent defect hunt.

## Independent recovery

Installed Wine VFP9 SP2 09.00.0000.7423 and shipped Microsoft BITCLEAR help
c3c6ff91-a5c0-47a8-b979-cb9020f13d5a.htm: Numeric BITCLEAR(value,position)
clears one bit, positions0..31 with0 rightmost, integer conversion first.
Numeric has exactly two arguments; binary optional start/count is a different
overload. Binary/type/arity policies are outside this slice. No binary inspection
or decompilation. Source input layout reuses a fixture harness, but all BITCLEAR
outputs are independently observed, not inferred from BITSET/Copperfin.

Initial plus3serial fresh bounded25s harness runs are retained byte-identical:
188lines/186calls. Value observations at positions0 AND1 recover bits otherwise
hidden by clearing one bit. Position observations use0 AND-1; absorbing value0
must still validate the position, while -1 distinguishes admitted positions.
All32 bits use -1 so each clear result is observable. Ten mixed controls include
0,32 ->11,7,1E20 ->6,7.9,1.9 ->5 and0,1E300 ->0.
Native fractions truncate toward zero; signed low32 aliases and huge finite/
infinity0 conversion precede the position0..31 check/error11. Exact64 and NaN
semantics are not claimed as native extension evidence. No VM started.

## Derived requirement and frozen verification plan

Parent RQ-CF-PRG-NUMERIC-BEHAVIOR-001 and owner #6776 require finite truncated/
exact signed32 value and position0..31 in COPPERFIN, otherwise catchable
localized11 before assignment/shift. Explicit VFP9 selects independently
observed low32/huge/infinity0 before position validation, including value0.
Exact64 conversion without Double round trip and NaN rejection derive from
parent safety intent. Defined uint32 clear-bit/AND complement and signed32
result in int64, no argument-driven work/allocation/persistence/cursor mutation.

Distinct helpers must remain unused by BITCLEAR through BOTH frozen complete
original GNU/Clang baselines. VR-5611-BITCLEAR-NUMERIC-001/002 cover independent
literal observations, exact64/nextafter/nonfinite boundaries, both modes and
selected sessions1/2, result kind, error11 and assignment12345 retention,
continuation/control5, preserved cursor/session/mode and reset1/COPPERFIN/closed
cursor. NaN is tested directly, not invented as a native expression. Safe
Logical/Character/Currency/empty coercions are preservation controls only;
unsafe other-type inputs reject by derived safety, not native type parity.
Two existing expression neighbors reused unchanged. Freeze all helpers/header/
tests/CMake/isolation BEFORE configuration, retain both full originals before
migration. Assertion repair/removal/weakening requires explicit refreeze and
BOTH original repeats. All original/fixed/broader/integration evidence remains
pending at this stage; this is not acceptance.

## Hazard, documentation, misuse and rollback

HZ-runtime-crash-01/HZ-data-corruption-01/HZ-doc-command-01 remain active at
registered severities; no hazard closure or safety-critical suitability claim.
Bounded medium misuse: VFP9 aliases can hide invalid untrusted values/positions;
default callers must catch11. DQ-5611-BITCLEAR-NUMERIC-001 procedural delta:
Numeric half-away rounding becomes truncation; default rejects wide/nonfinite
inputs; VFP9 observes low32/huge/infinity0 then checks the position even after0.
No KBX exception applied. DV-5611-BITCLEAR-NUMERIC-001 requires completed
development self-review and automated GNU/Clang walkthrough:7.9,1.9 ->5,
-1,31.9 ->2147483647,0,32 ->11 with result12345 unchanged and continuation5,
selected context and reset preserved. Exact64/NaN/nextafter/both modes must pass.
Self-review/walkthrough are pending, not independent-human/high-hazard signoff.
Rollback: revert only this slice, retain native/original/fixed evidence, correct
guidance and notify affected integrators if incorrect. Type/binary/arity/other
functions, full native/platform/leak/release/family/parent acceptance separate.

## Completed bounded local verification and development assurance

Both complete unchanged originals retained BEFORE migration: GNU14.56s and
Clang25.42s, actual206direct/800public,552 byte-identical selected failures
(252error/272result/28kind;406COPPERFIN/146VFP9), zero unrelated or sanitizer
diagnostics. All six frozen inputs and original dispatcher/shared platform
unchanged before BITCLEAR-only migration; no assertion repair/removal/weakening
or refreeze occurred. Full raw logs/hashes and chronology in baseline-audit.md.

BITCLEAR-only migration passed GNU focused2/2 in14.72s and broader19/19 in
461.97s (older Numeric/NULL/bit siblings/Collection/localization/seven contracts).
Safety contract kept1200s cap. Fixed Clang ASan/UBSan/float-cast-overflow11/11
PASS190.94s, no diagnostics/leaks disabled. Actual206direct/800public and sibling
counters retained; all six frozen inputs/shared platform unchanged. Private
RAM-backed TMPDIR and serial low-priority builds/tests used without workflow/
test-definition timeout changes. Timings are observations, not disk/persistence/
timeout-optimum qualification. No VM started.

Implementation-agent delegated development self-review completed2026-10-09:
fresh native literals and parent-derived safety compared with checked helpers
and BITCLEAR-only dispatch. Admission precedes narrowing/shift; exact64 avoids
Double round trips, NaN rejects, native infinity0 applies only in explicit VFP9.
Even value0 validates the later position. Defined uint32 AND complement and
signed32 arithmetic preserve int64 kind; no argument-driven work/persistence.
Shared helpers/siblings remain unchanged; safe other coercions are preservation
controls, not native type/Currency/binary/arity parity.

DV-5611-BITCLEAR-NUMERIC-001 automated GNU/Clang walkthrough passed:7.9,1.9 ->5;
-1,31.9 ->2147483647; invalid late32 after0 yields11 with nResult12345 intact,
continuation/control5 and cursor/session/mode retained, reset1/COPPERFIN/closed
cursor. Direct NaN/nextafter/exact64 controls and both modes/sessions pass.
DQ/DV bounded medium-misuse development self-review/walkthrough/rollback is
complete, not independent-human/high-hazard sign-off or closure of active
registered hazards. Signed postpush exact-head verification, actual clean
external review/required hosted checks/resolved conversations remain integration
gates. No type/binary/arity/full-native/platform/leak/release/family/parent
acceptance; BITTEST still needs its own bounded evidence.
