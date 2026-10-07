# Installed VFP9 TAG ordinal conversion evidence

Observed 2026-10-07 with installed VFP9 09.00.0000.7423 through the unchanged
owner-installed /home/rich/bin/vfp9-probe. Three fresh serial runs agree on
142 lines: 136 calls (68 operands in ordinal-first/index-file-first forms),
two omitted controls, VERSION, before/after state and successful cleanup.
The isolated native table has three distinguishable FIRST/SECOND/THIRD tags;
active SECOND order, record count/pointer, guard payload and data session stay
unchanged. No binary inspection, VM, external backend or system change.
The initial exploratory run stopped on an uncaught omitted control; it is not
acceptance evidence. Its precisely identified owned scratch directory was
moved to recoverable trash. The corrected complete runs clean their own files.

## Requirements and boundaries

RQ-CF-PRG-TAG-ORDINAL-NUMERIC-001 derives from admitted owner-authored
#5611/#6776, RQ-CF-PRG-NUMERIC-BEHAVIOR-001, HZ-runtime-crash-01 and
HZ-data-corruption-01. Default COPPERFIN requires finite Numeric, truncates
toward zero and admits positive ordinals up to 32767, retaining native's
raw-positive ceiling (32767.9 rejects before truncation). Zero/sub-unit and
negative ordinals reject catchable localized 11 before the tag callback.
This preserves correctly written ordinary legacy calls, while rejecting
out-of-range conversion rather than relying on host casts.

Explicit VFP9 rejects raw positive values above 32767, then converts through
signed low 32 bits and requires 1..32767. Native -4294967295/-4294967294
select FIRST/SECOND; positive 4294967297/8 reject, ruling out symmetric
wrapping. -4294934530/-4294934529 are admitted empty ordinals 32766/32767,
whereas -4294934528 rejects. Huge/infinite values reject because their
indefinite-zero alias is outside the domain. Native empty results cannot
independently prove each admitted missing ordinal's exact value; those exact
conversion values use documented derived modular policy, not an existence claim.

Exact int64/uint64 avoid floating round trips. Positive raw ceiling applies
before low-bit conversion; negative exact int64 modular aliases are a derived
extension policy. NaN rejects in both modes as derived safety, not native NaN
evidence. No argument-sized allocation or loop is introduced.
Other current coercions retain checked minimum-one/truncation and callback
results, not native type parity. Native Logical/NULL/Character and Currency
$0.5 reject 11; current coercions are intentionally preserved here (#7058).
Native TAG() returns current SECOND, while TAG(indexFile) rejects 11; current
default/routing remains separately controlled, not repaired by ordinal conversion
(#7057). Both separate issues were filed against main 6297dd2367318dbbd7db745271175ef872cc27ee;
neither admits production work or originates from current-code expectations.
TAG routing, index metadata/backend/result/type/arity, table/session lifecycle,
and adjacent KEY/DESCENDING conversion sites are outside this bounded slice.

## Reproduction and identities

~~~sh
/home/rich/bin/vfp9-probe tests/fixtures/vfp9-tag-ordinal-numeric-observation/probe.prg
sha256sum tests/fixtures/vfp9-tag-ordinal-numeric-observation/probe.prg tests/fixtures/vfp9-tag-ordinal-numeric-observation/probe.out
~~~

Source SHA-256: a3c8d0e70ec57c70c8b01d8c2611846995461c94d70f3357c9348361ca9bb2ad.
Output SHA-256: 3721ea80d774a2e8e859015b130ca4b6f52d0a5221b0132021b1f3904009b1dd.
VR-5611-TAG-NATIVE-001: three complete matching 142-line runs.

~~~sh
cmake -S . -B /home/rich/temp/copperfin-tag-ordinal-5611-build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS_DEBUG='-O0 -g1'
cmake --build /home/rich/temp/copperfin-tag-ordinal-5611-build --target test_prg_engine_numeric_behavior test_prg_engine_seek_index test_prg_engine_arrays test_prg_engine_string_math_functions -j2
ctest --test-dir /home/rich/temp/copperfin-tag-ordinal-5611-build --output-on-failure -R '^test_prg_engine_(numeric_behavior|seek_index|arrays|string_math_functions)$' -j1
cmake -S . -B /home/rich/temp/copperfin-tag-ordinal-5611-sanitize -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_CXX_FLAGS_DEBUG='-O0 -g1 -fsanitize=address,undefined,float-cast-overflow -fno-sanitize-recover=all' -DCMAKE_C_FLAGS_DEBUG='-O0 -g1 -fsanitize=address,undefined,float-cast-overflow -fno-sanitize-recover=all'
cmake --build /home/rich/temp/copperfin-tag-ordinal-5611-sanitize --target test_prg_engine_numeric_behavior test_prg_engine_seek_index test_prg_engine_arrays test_prg_engine_string_math_functions -j2
ctest --test-dir /home/rich/temp/copperfin-tag-ordinal-5611-sanitize --output-on-failure -R '^test_prg_engine_(numeric_behavior|seek_index|arrays|string_math_functions)$' -j1
~~~

Run runtime suites SERIAL across build directories. Hosted cross-platform
checks and clean exact-head review remain PR merge gates.

## Verification architecture and documentation controls

Independent expected columns cover 59 native Numeric operands plus exact,
adjacent, NaN and current-coercion direct controls. Fresh synthetic PRG cases
use both argument forms, indexed and empty guard cursors, both modes, caught
error/result/message, before/after order/record/payload/session/mode and cleanup.
Synthetic IDX/CDX metadata is independent test input, not native format
qualification. Omitted and other-type controls prevent scope drift; they do
not originate compatibility requirements. Runtime suites run SERIAL across
normal and sanitizer build directories.

DQ-5611-TAG-CONVERSION-001: truthful native/derived boundaries and checked
pre-callback admission, independent verification, serial execution and rollback.
Development procedure misuse severity is medium: unchecked conversion or false
ordinal aliases can crash the runtime or misidentify the index used downstream.
The parent crash/data-corruption hazards are mitigated by range checks, exact
integers, localized catchability and independent order/record/payload controls.
Procedural delta: replace only the two ordinal casts with an operation-specific
checked helper. Walkthrough: 1.9 selects FIRST; explicit VFP9 -4294967294 selects
SECOND, while default rejects 11. 32767.9 rejects in both modes. Rejection
leaves order, pointer, guard payload, session and mode unchanged; normal
lookups still work before cleanup resets mode and closes the owned cursor.
Rollback: reviewed revert retaining regressions, correct requirements/coverage/
handoff/release records and notify the owner before reusing incorrect guidance.
Development self-review and automated evidence are not independent human
review, release qualification or full TAG/type/routing/native index parity.

VR-5611-TAG-CONVERSION-001: 182 direct checks and 560 fresh cases/2580 PRG rows.
GCC unchanged-dispatch baseline fails exactly 346 selected output/error/message
assertions in Numeric 49.16s (four suites 51.46s). Every failed case has identical
setup/guard/after/cleanup and call-state suffix (zero mismatches); all direct,
omitted/coercion controls, older rows and three neighbors pass.
Clang baseline reports 21 selected semantic failures then actual float-cast-
overflow at expression.inl:1798:67 for 1E20 outside unsigned long. Numeric 1.32s;
three neighbors pass, four suites 10.65s. No completed sanitizer Numeric baseline
or unexecuted-case success is claimed. Both baseline binaries include the new
helper/tests but retain the original two casts at main 6297dd236.
The fix also avoids the already tracked shared Numeric formatter (#6997) for
Numeric first operands that cannot designate an index file. No shared formatter,
type/omitted/backend/index behavior or adjacent conversion is repaired.

VR-5611-TAG-CONVERSION-002: fixed GCC 15.2 Debug passes 4/4 in 52.23s
(Numeric 49.93s). Fixed Clang 21.1.8 ASan/UBSan/float-cast-overflow passes
4/4 in 248.87s (Numeric 239.68s), without diagnostics. All 182 direct checks
and 560 fresh cases/2580 PRG rows pass unchanged independent expectations.
Native source/output hashes remain unchanged.
VR-5611-TAG-CONTRACTS-001: six locale-install, contributor-signoff,
changelog assembler/fragment-validation, agent-channel and native-isolation
contracts pass 6/6 initially in 4.93s and finally in 4.39s after documentation
completion; 165 fragments are valid.

## Completed development documentation review

DV-5611-TAG-CONVERSION-001: implementation-agent development self-review
completed 2026-10-07. Reviewed the selected helper/expression/test diff,
complete source/output and three matching native runs, all 59 independent
Numeric expected columns against both native forms (118 comparisons, zero
mismatches), exact/NaN/adjacent boundaries and safe full-size_t other-coercion
controls, 560 fresh error/result/message/order/record/payload/session/mode/
cleanup cases, GCC and sanitizer baseline-to-fixed evidence, catalog-backed
11, docs/22 and docs/32 bidirectional mappings, medium-severity misuse,
procedural delta, walkthrough, rollback and separate #7057/#7058 gaps.
The reviewer is the implementation agent; this development self-review and
automated/native observations do not constitute independent human review,
release qualification or full TAG/native-index/type/routing parity.
Clean exact-head Claude or authorized Codex/owner-terminal review, all required
checks and resolved conversations remain separate merge gates.
