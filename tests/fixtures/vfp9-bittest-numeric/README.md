# BITTEST Numeric/exact value and position

RQ-CF-PRG-BITTEST-NUMERIC-001 mapped BEFORE helpers/tests or migration.
Bounded admitted #5611/#6776/#6871 slice at origin/main
53cf42c99e61491598ee4ae307d6c10ca5ccdd7d; no independent defect hunt.

## Independent recovery

Installed Wine VFP9 SP2 09.00.0000.7423 and shipped Microsoft BITTEST help
55285ec7-91ce-480f-ae2b-c3c41c36032e.htm describe Numeric value/position
integer conversion, positions0..31 with0 rightmost and a Logical result.
Binary/type/arity are separate. No binary inspection/decompilation or VM.
A reused probe harness supplies inputs only; BITTEST outputs are freshly
observed, never inferred from BITCLEAR/BITSET or Copperfin.

Initial plus3serial fresh bounded25s runs match byte-for-byte:
1304lines/1302calls. All32 bits of each of36value inputs reconstruct the
converted signed32 value independently, including sign-bit boundaries.
36position inputs use0,-1,1431655765 (alternating bits) so zero never hides
invalid admission and differing converted-position parity is observable.
32alternating-bit checks plus10mixed controls retain Logical expectations:
5,1 ->false;7.9,1.9 ->true;0,32 ->11;7,1E20 ->true;0,1E300 ->false.
Fractions truncate toward zero; signed low32 aliases and huge finite/infinity0
convert BEFORE position0..31 admission/error11, even after value0.

After fixed GNU verification, a supplemental installed-only one-bit-mask probe
independently disambiguates every exact position index for the SAME36 original
inputs:32 literal masks each,1152calls/1154lines. Initial plus3fresh bounded
Wine runs match byte-for-byte. Twenty-six admitted inputs each have exactly
one true mask (832 Logical calls); ten rejected inputs each error11 on all32
masks (320 calls). Fractional/low32/huge/infinity indices agree with the existing
frozen expectations; no test input, expectation, assertion or case count changes.
This strengthens position-index evidence after GNU, not a claim it preceded
initial mapping/helpers/tests. Source/results/hashes retained in baseline-audit.md.

## Derived requirements and frozen verification

Parent RQ-CF-PRG-NUMERIC-BEHAVIOR-001 and owner policy require default finite
Numeric/exact signed32 value and truncated position0..31 or localized catchable11.
Explicit VFP9 keeps independently observed low32/huge/infinity0, then validates
the position. Exact64 avoiding Double round trips and NaN rejection are derived
safety boundaries, not native syntax/evidence. Result must remain Logical.
A checked uint32 shift and bit-test add no argument-driven allocation/work,
persistence, cursor or session mutation.

Distinct helpers remain unused through BOTH complete frozen original GNU/
Clang baselines. VR-5611-BITTEST-NUMERIC-001/002 cover206direct and5264freshpublic
cases, native literal1302rows plus14extension/preservation controls, both modes/
selected sessions1/2, Logical kind, error11, assignment12345 retention,
continuation/controltrue, cursor/session/mode preservation and reset1/COPPERFIN/
closedcursor. Safe Logical/Character/Currency/empty are preservation only;
unsafe other-type containment is derived safety, not native type parity.
Two existing expression neighbors unchanged. All helper/header/tests/CMake/
isolation frozen BEFORE configuration; no repair/removal/weakening without
explicit refreeze and BOTH original reruns. See baseline-audit.md.
Original/fixed/broader/integration evidence remains pending at freeze.

The explicit harness-only refreeze in baseline-audit.md retains all cases and
existing assertions, with206direct in the default executable invocation and
four1316-public CTests: numeric_copperfin_session1/2 and numeric_vfp9_session1/2
(full names prefixed test_prg_engine_bittest_). Public executable arguments
are COPPERFIN|VFP9 followed by1|2; invalid arguments fail. Run the full focused
regex ^test_prg_engine_bittest_(numeric.*|neighbors)$ serially to cover all
206direct/5264public and unchanged neighbors. BOTH complete original baselines
must be rerun after this refreeze before production migration; no expectation
is repaired, removed or weakened and incomplete runs are not clean evidence.

## Hazard, documentation, misuse and rollback

HZ-runtime-crash-01/HZ-data-corruption-01/HZ-doc-command-01 remain active at
registered severities, not closed or safety-critical suitability claims.
Bounded medium misuse: VFP9 aliases can hide invalid input; default callers
must catch11. DQ-5611-BITTEST-NUMERIC-001 procedural delta: Numeric rounding
becomes truncation; default rejects wide/nonfinite inputs; explicit VFP9 retains
observed low32/huge/infinity0 but still validates position even for value0.
No KBX exception applied. DV-5611-BITTEST-NUMERIC-001 requires completed delegated
development self-review and automated GNU/Clang walkthrough:
7.9,1.9 ->true; -2147483648,31.9 ->true;0,32 ->11 with assignment12345 intact,
continuation/controltrue/context preserved/reset verified. Exact64/NaN/nextafter/
bothmodes must pass. Self-review/walkthrough pending; not independent-human/
high-hazard signoff. Rollback: revert only this slice, retain evidence, correct
guidance and notify affected integrators if incorrect.
Type/Currency/binary/arity/otherfunctions/fullnative/platform/leak/release/family/
parent acceptance remains separate.

## Completed bounded local evidence

BOTH complete refrozen originals precede BITTEST-only migration: GNU144.36s/
Clang324.57s,206direct+4x1316public and2954 byte-identical selected-only failures,
zero other assertions; incomplete earlier runs preserved as incomplete. No
frozen expectations/assertions changed or weakened by the harness-only refreeze.
Fixed GNU6/6 focused PASS149.78s, broader25/25 PASS631.19s; fixed Clang17/17
PASS526.10s with ASan/UBSan/float-cast-overflow and no diagnostics/leaks disabled.
All cases complete; unchanged neighbors, older Numeric/NULL/bitfamily and GNU
localization/seven contracts pass. Timings are observations, not optimal-timeout,
disk/persistence/platform/release qualifications. Full raw logs/hashes and
completed bounded medium-risk delegated development self-review/automated DQ-DV
walkthrough are retained in baseline-audit.md; earlier pending statements above
describe the initial freeze, not current local status. No independent-human/
high-hazard signoff. Exact-head integration gates remain before merge; no
family or parent closure from this Numeric-only slice.
