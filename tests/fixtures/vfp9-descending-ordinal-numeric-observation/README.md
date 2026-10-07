# Installed VFP9 DESCENDING ordinal conversion evidence

Observed 2026-10-07 with installed VFP9 09.00.0000.7423 through the unchanged
owner-installed /home/rich/bin/vfp9-probe. Three complete fresh serial runs
agree on 142 lines: 136 ordinal calls (68 operands, current/explicit-alias
forms), two active-form controls, VERSION, before/after state and cleanup.
The isolated native table has FIRST ascending, SECOND descending, THIRD
ascending; active FIRST ascending. Record count/pointer, id31/guard payload,
data session1 and active direction stay unchanged. Every completed probe
removes its owned files. No VM, backend or system changes.

## Requirements recovered before implementation

RQ-CF-PRG-DESCENDING-ORDINAL-NUMERIC-001 derives from admitted #5611/#6776,
RQ-CF-PRG-NUMERIC-BEHAVIOR-001, HZ-runtime-crash-01/HZ-data-corruption-01.
Default COPPERFIN finite Numeric truncates toward zero and admits1..32767,
with native raw-positive ceiling:32767.9 rejects11 before truncation.
Zero/sub-unit/negative reject localized catchable11 before the direction
callback. Correct ordinary legacy calls retain their native conversion.

Explicit VFP9 first rejects raw positive values above32767, then uses
signed low32 conversion and requires1..32767. Independent DESCENDING
observations recover2.49/2.5/2.9 and -4294967294 selecting SECOND (.T.).
Positive4294967297/8/9 reject; negative -4294967295/-4294967295.9 return.F.
Huge/infinite conversions reject11. NaN and exact int64/uint64 boundaries
are derived safety/extension policy, not native extension observations;
exact integers must avoid a floating round trip. No argument-sized allocation
or loop. False results cannot distinguish ascending/missing tags or prove
exact missing ordinals32766/32767; those follow derived modular policy.

An initial fixture activated SECOND with runtime ASCENDING; every admitted
ordinal returned.F., confounding the intended direction distinctions.
The final fixture activates FIRST instead, leaving SECOND's creation
direction independently observable. The initial run is not counted as an
alias-distinguishing final fixture run. Existing recovered RQ-CF-PRG-019 and
synthetic runtime-override controls are context, not proof of full native
metadata/direction parity; no callback or format change is authorized here.

Other-type and active-form routing are preservation controls, never their
own compatibility requirements. Native Logical/NULL/Character/$0.5 ordinals
reject11; $1.5/$2.5 return.F./.T. DESCENDING() returns active.F.;
DESCENDING('descprobe') rejects11. Current differences must be verified and
recorded separately without expanding this conversion slice.
Current other coercions belowone retain engaged zero (false selection),
positive representable conversions retain the checked full size_t domain.
Unsafe positive conversions reject11. Single-argument routing, optional
work-area formatter, index/backend metadata/direction/runtime overrides,
arity and session/table lifecycle stay separate. TAG/KEY unchanged.

## Native identities and reproduction

~~~sh
/home/rich/bin/vfp9-probe tests/fixtures/vfp9-descending-ordinal-numeric-observation/probe.prg
sha256sum tests/fixtures/vfp9-descending-ordinal-numeric-observation/probe.prg tests/fixtures/vfp9-descending-ordinal-numeric-observation/probe.out
~~~

Source SHA-256:0312f20f354611c5f25d6460841a7b9891b55e91170a9663b437efc0422b2448.
Output SHA-256:f40ddd5c7e8633381cf49574843a1346684a3042eef12dbff209255b45f59d71.
VR-5611-DESCENDING-NATIVE-001: three complete matching136-call/142-line runs.
59 independent Numeric columns/118 comparisons against both forms agree;
missing exact ordinals are derived, not inferred from false native results.

## Verification architecture

182 direct checks include59 independent Numeric columns and32 exact/NaN/
adjacent/current-coercion boundaries in each mode. Fresh synthetic two-tag
CDX plus a preceding global IDX distinguishes per-file ordinal2 (.T.) from
global ordinal2 (the first CDX tag). Active ASCTAG runtime DESCENDING remains
.T., independently distinct from its persisted.F. callback result. Both
current/explicit-alias forms, indexed/empty cursors and both modes assert
result/error/message, unchanged order/direction/pointer/name/alias/session/
mode and reset/cleanup. 576freshcases/2652PRGrows; synthetic encodings do not qualify
native CDX/IDX formats. Character1E300 rejects; representable1E19 preserves
full-size_t on64-bit hosts. Runtime suites must run SERIAL across builddirs.
Exact-head review/required hosted checks/resolved threads gate merge.

~~~sh
cmake -S . -B /home/rich/temp/copperfin-descending-ordinal-5611-build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS_DEBUG='-O0 -g1'
cmake --build /home/rich/temp/copperfin-descending-ordinal-5611-build --target test_prg_engine_numeric_behavior test_prg_engine_ordered_iteration test_prg_engine_seek_index test_prg_engine_arrays test_prg_engine_string_math_functions -j4
ctest --test-dir /home/rich/temp/copperfin-descending-ordinal-5611-build --output-on-failure -R '^test_prg_engine_(numeric_behavior|ordered_iteration|seek_index|arrays|string_math_functions)$' -j1
cmake -S . -B /home/rich/temp/copperfin-descending-ordinal-5611-sanitize -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_CXX_FLAGS_DEBUG='-O0 -g1 -fsanitize=address,undefined,float-cast-overflow -fno-sanitize-recover=all -fno-omit-frame-pointer' -DCMAKE_EXE_LINKER_FLAGS='-fsanitize=address,undefined,float-cast-overflow'
cmake --build /home/rich/temp/copperfin-descending-ordinal-5611-sanitize --target test_prg_engine_numeric_behavior test_prg_engine_ordered_iteration test_prg_engine_seek_index test_prg_engine_arrays test_prg_engine_string_math_functions -j4
copperfin_desc_verify_tmp=$(mktemp -d /home/rich/temp/copperfin-descending-ordinal-5611-sanitize/verify-tmp.XXXXXX)
TMPDIR="$copperfin_desc_verify_tmp" ctest --test-dir /home/rich/temp/copperfin-descending-ordinal-5611-sanitize --output-on-failure -R '^test_prg_engine_(numeric_behavior|ordered_iteration|seek_index|arrays|string_math_functions)$' -j1
~~~

Separate gaps: #7063 ordinal type admission and #7064 single-argument alias
routing, filed against exact mainc5b6875c85b28b781ad397834d1654a4d84c2502.
Neither receives admission labels or production changes in this slice.

VR-5611-DESCENDING-CONVERSION-001: corrected GCC original-dispatch baseline
fails350 selected Numeric/unsafe-coercion assertions, with independently parsed
zero setup/after/cleanup or call-state-suffix mismatches. Numeric59.91s,
five suites62.69s. Direct checks, ordinary other-type/active-form controls,
older rows and four neighbors pass. Clang reports21 selected assertions then
actual float-cast-overflow at expression.inl:1766:56 for1E20 outside unsigned
long; Numeric1.40s,five suites12.60s,four neighbors pass. No completed
sanitizer Numeric baseline or unexecuted-case success is claimed. Both
baseline binaries contain the new unused helper/regressions but original
mainc5b6875c8 DESCENDING dispatch.

Initial builds failed because the private synthetic writer needed fstream;
the missing include was corrected in both binaries before runtime baselines.
An earlier synthetic FIRST/SECOND fixture with unlike NAME/UPPER(NAME)
expressions failed setup (ERR1/no active order), confounding462 GCC failures
and the first sanitizer run. Those are not accepted conversion evidence.
The corrected fixture uses the existing verified ASCTAG/DESCTAG NAME layout,
preserving the preceding IDX. Both baselines were rebuilt and rerun; only
the corrected baseline above establishes state-preserving failures.


First fixed Clang acceptance attempt: Numeric287.68s,total298.25s, 22
assertions fail in older SQLPREPARE/SQLEXEC state/event controls, while all
DESCENDING/direct rows and four neighbors pass and no sanitizer diagnostic
is emitted. The complete negative record is first-fixed-sanitizer-failure.log,
SHA-256 e346bb0629d14a86228c46c1a7c21cab8e9f39e571d14704703ac5b4f436221e.
The process snapshot after the run found no competing Numeric process.
Shared fixed-temp-file interference is suspected, not proven; no unrelated
SQL code is changed. Rerun uses the identical binary and a fresh private
TMPDIR under the owned sanitizer build. Do not erase this failed attempt or
claim its full suite passed. The isolated rerun completes all five suites without diagnostics; its result
below does not prove the failed run's root cause or erase its negative record.

VR-5611-DESCENDING-CONVERSION-002: fixed GCC15.2 Debug passes5/5 in62.89s
(Numeric57.95s). Fixed Clang21.1.8 ASan/UBSan/float-cast-overflow, identical
binary with private TMPDIR verify-tmp.wHtEko, passes5/5 in360.20s
(Numeric348.51s), no diagnostics. Only this Numeric process was present in
the sampled process snapshots; no cross-directory runtime suites overlapped
within this task. Six contracts pass6/6 in4.29s;167validfragments.

DQ-5611-DESCENDING-CONVERSION-001: truthful native/derived requirements,
checked pre-callback admission, independent stateful verification, serial
runtime execution/private temporary roots and reviewed rollback.
Development procedure misuse severity is medium: unsafe Numeric narrowing can
crash the runtime or misreport an index direction consumed downstream.
Parent HZ-runtime-crash-01/HZ-data-corruption-01 controls are checked bounds,
exact integers, localized catchability and independent guard/state checks.
Procedural delta: replace only the index-file-first second-argument ordinal
cast; preserve active-form routing, other-type coercion, designator formatter,
callback, index metadata/runtime overrides, arity and lifecycle. Test-fixture
construction and failed/confounded attempts are explicitly retained; no
native-format/type/routing parity inferred from synthetic controls.

Walkthrough: synthetic1.9 selects persisted ascending ASCTAG (.F.) although
its active runtime direction is.T.;2.9 selects persisted descending DESCTAG
(.T.), ignoring the preceding global IDX for this per-file ordinal.
Explicit VFP9 -4294967294 selects DESCTAG; default rejects11;32767.9 rejects11
in both modes. Failures leave active ASCTAG runtime.T., pointer3/CHARLIE,
recordcount3, alias, session1 and mode unchanged. Empty cursor controls retain
ALPHA/pointer1. Mode resets COPPERFIN and the owned alias closes.
Rollback: reviewed revert retaining regressions, correct requirements/coverage/
handoff/release evidence, and notify the owner before reusing wrong guidance.

DV-5611-DESCENDING-CONVERSION-001: completed2026-10-07 implementation-agent
development self-review, with independent expected columns/native comparison,
corrected baseline failure-state audit, source/delta/misuse/walkthrough review,
normal/sanitizer acceptance and six automated contracts described above.
This is not independent qualified-human verification, formal certification,
release/platform qualification or full native CDX/IDX/type/routing parity.
