# Installed VFP9 KEY ordinal conversion evidence

Observed 2026-10-07 with installed VFP9 09.00.0000.7423 through the unchanged
owner-installed /home/rich/bin/vfp9-probe. Three complete fresh serial runs
agree on 142 lines: 136 calls (68 operands in ordinal-first/index-file-first
forms), two omitted controls, VERSION, before/after state and cleanup.
The isolated native table has three independently distinguishable key
expressions ID/MARKER/-ID. Active SECOND, record count/pointer, id31/guard
payload and data session1 remain unchanged. Owned native files are removed
by every completed probe. No VM, external backend or system change.

## Requirements recovered before implementation

RQ-CF-PRG-KEY-ORDINAL-NUMERIC-001 derives from admitted owner-authored
#5611/#6776, RQ-CF-PRG-NUMERIC-BEHAVIOR-001, HZ-runtime-crash-01 and
HZ-data-corruption-01. Default COPPERFIN requires finite Numeric, truncates
toward zero and admits 1..32767 with the observed raw-positive ceiling:
32767.9 rejects11 before truncation. Zero/sub-unit/negative reject catchable
localized11 before the key callback. This retains correctly written ordinary
legacy calls without undefined host casts.

Explicit VFP9 first rejects raw positive values above32767, then uses
signed low32 conversion and requires1..32767. Independent KEY observations
prove -4294967295/-4294967294 select ID/MARKER and -4294967295.9 selects ID;
positive4294967297/8 reject. Negative -4294934530/-4294934529 return empty
missing-key results, whereas -4294934528 rejects11. Exact missing ordinals
32766/32767 follow derived modular policy: empty results alone cannot prove
the exact converted ordinal. Huge/infinite indefinite-zero aliases reject11
because zero is outside the admitted domain. This KEY evidence was collected
independently; similarity to TAG does not replace the KEY oracle.

NaN rejection and exact int64/uint64 admission/modular boundaries are derived
safety/extension policy, not native extension evidence. Exact integers must
avoid a floating round trip. No argument-sized allocation or loop.

Other current coercions, omitted routing, index metadata/backend/results/type/
arity and table/session lifecycle are preservation controls, not compatibility
requirements. Native Logical/NULL/Character/$0.5 ordinals reject11; $1.5/$2.5
select1/2. KEY() returns active MARKER; KEY(indexFile) rejects11. Current-code
differences must be verified with independent controls and filed separately
against the active main revision, without repairing them in this slice.
For current other-type coercions retain the existing less-than-one empty
selection, not TAG's minimum-one rule. Representable positive conversions
must remain checked over the full size_t domain; unsafe conversions require
a documented stable result/error, not a cast. DESCENDING and TAG are separate.

## Reproduction and identities

~~~sh
/home/rich/bin/vfp9-probe tests/fixtures/vfp9-key-ordinal-numeric-observation/probe.prg
sha256sum tests/fixtures/vfp9-key-ordinal-numeric-observation/probe.prg tests/fixtures/vfp9-key-ordinal-numeric-observation/probe.out
~~~

Source SHA-256: 9f95910a651ca7ab0dfaef25bd99d96b4501791f2884b9058d627347dc2d514c.
Output SHA-256: 4202e62936e8d44aac407aef13e5cb206c54b8108d1f0390282d4d17ac7ca0a4.
VR-5611-KEY-NATIVE-001: three complete matching136-call/142-line runs.

~~~sh
cmake -S . -B /home/rich/temp/copperfin-key-ordinal-5611-build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS_DEBUG='-O0 -g1'
cmake --build /home/rich/temp/copperfin-key-ordinal-5611-build --target test_prg_engine_numeric_behavior test_prg_engine_seek_index test_prg_engine_arrays test_prg_engine_string_math_functions -j4
ctest --test-dir /home/rich/temp/copperfin-key-ordinal-5611-build --output-on-failure -R '^test_prg_engine_(numeric_behavior|seek_index|arrays|string_math_functions)$' -j1
cmake -S . -B /home/rich/temp/copperfin-key-ordinal-5611-sanitize -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_CXX_FLAGS_DEBUG='-O0 -g1 -fsanitize=address,undefined,float-cast-overflow -fno-sanitize-recover=all -fno-omit-frame-pointer' -DCMAKE_EXE_LINKER_FLAGS='-fsanitize=address,undefined,float-cast-overflow'
cmake --build /home/rich/temp/copperfin-key-ordinal-5611-sanitize --target test_prg_engine_numeric_behavior test_prg_engine_seek_index test_prg_engine_arrays test_prg_engine_string_math_functions -j4
ctest --test-dir /home/rich/temp/copperfin-key-ordinal-5611-sanitize --output-on-failure -R '^test_prg_engine_(numeric_behavior|seek_index|arrays|string_math_functions)$' -j1
~~~

Run runtime suites SERIAL across build directories. Exact-head review,
required hosted checks and resolved conversations remain separate merge gates.

## Verification architecture and documentation controls

Independent columns cover 59 Numeric operands observed through KEY, plus
32 exact/NaN/adjacent/current-coercion direct controls in each mode. All
118 comparisons against both native forms match. Exact absent ordinals
are derived modular policy, not independently proven by empty native results.
Fresh synthetic IDX/CDX cases use both forms, indexed and empty cursors,
both modes, caught error/result/message and unchanged order/record/payload/
alias/session/mode/reset/cleanup. They do not qualify native file formats.
Other-type/omitted controls preserve current behavior without originating a
compatibility requirement. Zero is a helper sentinel for the preserved empty
other-type selection; missing means unsafe conversion, not omitted routing.
Character '1E300' rejects safely; representable '1E19' retains the full size_t
domain on 64-bit hosts rather than narrowing through signed int64.
The helper and dispatch have no argument-sized allocation or loop.

DQ-5611-KEY-CONVERSION-001: truthful native/derived boundaries, checked
pre-callback admission, independent verification, serial execution and rollback.
Development procedure misuse severity is medium: unchecked conversion can
crash the runtime or misidentify a key expression used downstream.
Parent HZ-runtime-crash-01/HZ-data-corruption-01 controls are bounds, exact
integer handling, localized catchability and independent state/payload checks.
Procedural delta: replace only the KEY ordinal conversion boundary in its
two forms; preserve callbacks, omitted/type routing, metadata and neighbors.
Numeric first operands cannot designate a file and bypass the already
tracked unsafe formatter (#6997); the shared formatter itself is unchanged.
Walkthrough: 1.9 selects NAME in the synthetic table; explicit VFP9
-4294967294 selects UPPER(NAME), while default rejects 11. Both modes reject
32767.9. Rejections leave order, record/payload, alias, session and mode intact.
Corrected neighboring KEY(0)/KEY(-1) tests catch 11 and still execute the
following ordinary KEY/TAGCOUNT controls. Cleanup resets mode and closes
the owned cursor.
Rollback: reviewed revert retaining regressions; correct requirements/coverage/
handoff/release records and notify the owner before reusing incorrect guidance.
Native KEY() active-key and KEY(indexFile) error behavior are separate #7060;
native Logical/NULL/Character/$0.5 admission is separate #7061.
Neither is repaired or owner-admitted by this conversion slice.

VR-5611-KEY-CONVERSION-001: 182 direct checks and 576 fresh cases/
2652 PRG rows. GCC original-dispatch baseline fails 353 selected Numeric/
unsafe-coercion output/error/message assertions in Numeric 51.13s (four suites
53.34s). All failed-case setup/after/cleanup and call-state suffixes match
independently (zero mismatches). Direct, ordinary coercion/omitted controls,
older rows and three neighbors pass. Clang baseline reports 21 selected
semantic failures, then actual float-cast-overflow at expression.inl:1842:57
for 1E20 outside unsigned long; Numeric 1.41s, four suites 10.29s.
Three neighbors pass. No completed sanitizer Numeric baseline or unexecuted-
case success is claimed. Both baseline binaries contain the new helper/tests
but retain main cb6bf96a0's original KEY dispatch.

Initial fixed Clang execution passed Numeric in 251.86s, arrays in 4.39s and
string/math in 2.16s without sanitizer diagnostics, but the seek/index
neighbor stopped on its old uncaught KEY(0)/KEY(-1) soft-empty expectations,
producing six downstream assertions. That expectation is contradicted by
the independent native evidence. The neighbor now catches/asserts 11,
preserving its later ordinary KEY/TAGCOUNT checks; both binaries rebuilt.
Final whole-suite evidence below is after that correction.

VR-5611-KEY-CONVERSION-002: final GCC 15.2 Debug passes 4/4 in 57.73s
(Numeric 55.46s). Final Clang 21.1.8 ASan/UBSan/float-cast-overflow passes
4/4 in 244.22s (Numeric 235.10s), without diagnostics. All 182 direct checks
and 576 fresh cases/2652 PRG rows pass the unchanged independent expected
columns. Native source/output hashes remain unchanged.
VR-5611-KEY-CONTRACTS-001: six locale-install, contributor-signoff,
changelog assembler/fragment-validation, agent-channel and native-isolation
contracts pass 6/6 initially in 4.79s and after documentation in 4.16s;
166 fragments valid.

## Completed development documentation review

DV-5611-KEY-CONVERSION-001: implementation-agent development self-review
completed 2026-10-07. Reviewed the selected helper/dispatch/test diff, complete
native source/output and three matching runs, 59 independent Numeric columns
against both forms (118 comparisons, zero mismatches), exact/NaN/adjacent
boundaries, checked full-size_t other coercions and zero sentinel semantics,
576 fresh error/result/message/order/record/payload/alias/session/mode/cleanup
cases, baseline-to-fixed GCC and sanitizer evidence, corrected Numeric error
expectations in the index neighbor, catalog-backed 11, docs/22/docs/32 reverse
mappings, medium-severity misuse, procedural delta, walkthrough, rollback
and separate #7060/#7061 gaps. The reviewer is the implementation agent:
development self-review and automated/native evidence are not independent
human review, release qualification or full KEY/native-index/type/routing
parity. Clean exact-head Claude or authorized Codex/owner-terminal review,
all required checks and resolved conversations remain separate merge gates.
