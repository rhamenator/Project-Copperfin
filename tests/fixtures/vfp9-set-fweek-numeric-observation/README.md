# Installed VFP9 SET FWEEK numeric observations

Recovered 2026-10-06 from installed Visual FoxPro 09.00.0000.7423 under Wine
using the unchanged owner-installed /home/rich/bin/vfp9-probe runner.
fweek.prg is complete clean-room source; fweek.out retains VERSION, 50 evaluated
operands, omitted reset, three WEEK consumer checks and final setting in 56
lines. Three fresh processes match; all three final outputs were compared byte-for-byte. The omitted
setting readback uses ALLTRIM only to remove display padding. Each attempt
begins with FWEEK 2. Cleanup restores FWEEK 1. No table, SQL/network/backend, VM, installer
or system change was used. Generated FXP is ignored; no proprietary binary
was inspected.

## Selected requirement and boundary

RQ-CF-PRG-SET-FWEEK-NUMERIC-001 derives from owner-approved #5611/#6776,
RQ-CF-PRG-NUMERIC-BEHAVIOR-001 and HZ-runtime-crash-01/HZ-data-corruption-01.
Only SET FWEEK TO's integer-setting dispatch is selected. Numeric operands
truncate toward zero into 1..3 (1.9 -> 1, 3.9 -> 3). Default COPPERFIN
rejects non-finite or out-of-domain input. Explicit VFP9 admits negative-only
signed-low-32 aliases before that domain (-4294967295 -> 1); positive wrapping
(4294967297), zero and huge/infinite indefinite zero remain errors.
Rejection raises localized catchable error 46 before setting mutation or
a successful SET event. Omitted operand resets to 1.

The independent 43-Numeric native table exposes actual settings, domain,
fractional truncation, negative aliases, positive non-wrapping and huge/infinite
rejection. Exact int64/uint64 keep low bits without a double round trip; NaN
rejects in both modes. These exact-integer/NaN policies derive from parent
checked-conversion and hazard requirements, not native extended/NaN evidence.
The operation-specific helper shares only the independently verified negative-
only signed-int32 conversion model used by SQLGETPROP, not SQL semantics.

Bare Numeric tokens enter Numeric conversion, not Character coercion.
Complete decimal literal syntax retains Numeric rejection when finite parsing
overflows or totally underflows (such as 1E999 or 1E-999). This lexical/range
policy derives from the selected domain/hazard contract, not additional native
literal-range observations. Quoted and malformed text retain existing fallback;
parenthesized operands retain their actual kind. Existing evaluation exceptions
still fall back to 1; evaluation/resumption is not redesigned. Other ordinary
Currency/logical/null/Character operands retain checked half-away rounding,
clamping/fallback to 1, not native type parity. Native controls instead raise
10 while retaining 2; separate gap #7010 against exact main
0b481f4eff562a56f27e2171e16da28b21eb4db9 is not admitted into this slice.
EPOCH, other settings and date/week algorithms remain separate.
There is no operand-magnitude-sized allocation or loop, or new backend.
The complete-literal check makes one linear source-text pass with constant state.

## Reproduction and identity

From repository root:

```sh
/home/rich/bin/vfp9-probe tests/fixtures/vfp9-set-fweek-numeric-observation/fweek.prg
sha256sum tests/fixtures/vfp9-set-fweek-numeric-observation/fweek.prg tests/fixtures/vfp9-set-fweek-numeric-observation/fweek.out
```

Source SHA-256: 51f5435e202b20bf3c73e1b5f1cd50ff3a86304c1fa84f846478c650a5471abd.
Output SHA-256: c9210c10965133e7d67682c9cba84411543482814da7859754cd65722b1cc6ee.
VR-5611-SET-FWEEK-NATIVE-001: three fresh outputs match; last two byte-for-byte.
Error 46 preserves 2; negative aliases select 1/3; omitted reset gives 1.

## Focused verification

144 direct calls cover 43 native Numeric and 29 adjacent/extended/NaN/coercion
cases in both modes. 188 fresh runtime cases cover bare and parenthesized
Numeric tokens, catchability, exact setting/preservation, WEEK's use of FWEEK,
omitted reset, mode reset and success-event suppression. Four additional cases
check diagnostic text/code/state in all shipped locales with scoped/restored
COPPERFIN_LOCALE. Runtime numeric tests run serially across build directories
because this suite and neighboring numeric tests own shared temporary paths.

```sh
cmake -S . -B /home/rich/temp/copperfin-set-fweek-5611-build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS_DEBUG='-O0 -g1'
cmake --build /home/rich/temp/copperfin-set-fweek-5611-build --target test_prg_engine_numeric_behavior test_prg_engine_relations test_prg_engine_database_lifecycle test_prg_engine_date_time_functions test_prg_engine_runtime_surface_functions test_localization -j4
ctest --test-dir /home/rich/temp/copperfin-set-fweek-5611-build -R '^test_prg_engine_(numeric_behavior|relations|database_lifecycle|date_time_functions|runtime_surface_functions)$' --output-on-failure
cmake -S . -B /home/rich/temp/copperfin-set-fweek-5611-sanitize -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_CXX_FLAGS_DEBUG='-O0 -g1 -fsanitize=address,undefined,float-cast-overflow -fno-omit-frame-pointer' -DCMAKE_EXE_LINKER_FLAGS='-fsanitize=address,undefined,float-cast-overflow'
cmake --build /home/rich/temp/copperfin-set-fweek-5611-sanitize --target test_prg_engine_numeric_behavior test_prg_engine_relations test_prg_engine_database_lifecycle test_prg_engine_date_time_functions test_prg_engine_runtime_surface_functions -j2
ctest --test-dir /home/rich/temp/copperfin-set-fweek-5611-sanitize -R '^test_prg_engine_(numeric_behavior|relations|database_lifecycle|date_time_functions|runtime_surface_functions)$' --output-on-failure
ctest --test-dir /home/rich/temp/copperfin-set-fweek-5611-build -R '^test_(locale_catalog_install_contract|native_test_isolation_contract|localization|changelog_fragment_assembler|changelog_fragments_valid|contributor_signoff_contract|agent_channel_contract)$' --output-on-failure
```

VR-5611-SET-FWEEK-REGRESSION-001: original FWEEK dispatch fails exactly
572 selected setting/error/event/locale/WEEK-consumer assertions in both GCC
(numeric 8.86s; four-suite run 10.22s) and Clang sanitizers (numeric 38.15s;
full run 43.96s). All helper checks, preserved coercions and existing Numeric
rows plus three neighbors pass before dispatch changes. No baseline sanitizer
diagnostic occurred.
Final GCC passes 4/4 in 10.23s (numeric 8.92s).
VR-5611-SET-FWEEK-SANITIZER-001: final Clang ASan/UBSan/float-cast-overflow
passes 4/4 in 44.17s (numeric 38.24s) without diagnostics.
Catalog/install, localization/isolation, signoff, changelog and live-channel
contracts pass 7/7 in 18.52s; all 148 fragments validate.
Fixture identity and git diff --check pass. Local evidence is not independent-
human, installed-GUI, complete SET evaluation or full date/week parity evidence.
Hosted exact-head checks/review remain separate integration gates.

## Broad validation setup correction

The first hosted head 2ea82582f passed the Numeric suite but both Linux GCC
and macOS Clang failed the runtime-surface suite with 27 cascading assertions.
Its setup used SET FWEEK TO 4 while expecting readback/restoration of 3,
an obsolete clamp contradicted by this native fixture's error 46. The setup
now uses valid 3; readback, new-session default 1 and restored value 3 remain
asserted. The dedicated Numeric cases retain error-46/state/event coverage
for 4 in both modes. No production change or unrelated expectation is needed.
VR-5611-SET-FWEEK-SURFACE-001: unchanged setup reproduces exactly 27 failures
locally in GCC (5.51s) and Clang sanitizers (23.22s), without sanitizer
diagnostics. Corrected setup plus Numeric/relations/database-lifecycle/date-time
passes 5/5 in GCC (15.26s; surface 5.36s; Numeric 8.52s).
Clang ASan/UBSan/float-cast-overflow passes the same 5/5 (66.03s; surface
23.13s; Numeric 36.93s) without diagnostics. Toolchain runtime runs are serial.
Seven catalog/localization/isolation/signoff/changelog/channel contracts pass
7/7 (19.36s), with 148 valid fragments. These setup results supplement rather
than replace the production fail-before/pass-after evidence above.

## Bare literal-range review correction

Review comment 4194174679 identified a failed finite parse of a complete
bare decimal Numeric token falling through to Character fallback 1. A FWEEK-
local complete decimal syntax check now preserves Numeric rejection through
the existing checked helper/error 46 before mutation/event publication.
The internal NaN rejection sentinel is not a parsed value or native observation.
No shared parser, other setter or expression-evaluation fallback is changed.

VR-5611-SET-FWEEK-LITERAL-RANGE-001: 50 additional fresh runtime rows cover
both signs, leading plus, exponent signs/case, leading/trailing decimal points,
total overflow/underflow, representable subnormal and zero controls. Quoted
out-of-range text, malformed numeric-like text and finite decimal controls
preserve their earlier behavior. With tests retained but pre-review f48a79dc1
dispatch unchanged, exactly 80 selected error/state/WEEK/event assertions fail
in GCC (8.77s) and Clang sanitizers (37.07s), with no sanitizer diagnostics.
Other existing/helper/control assertions pass. This supplements the 188
original runtime cases (238 total); direct calls and four locale cases are
unchanged. These are derived lexical/range safety cases, not native evidence.
After the FWEEK-only classification correction, all five selected suites pass
in GCC (15.61s; Numeric 8.74s; surface 5.55s) and Clang
ASan/UBSan/float-cast-overflow (69.95s; Numeric 40.69s; surface 23.35s),
without diagnostics. Runtime runs are serial. All seven catalog/localization/
isolation/signoff/changelog/channel contracts pass 7/7 (19.66s), and all
148 fragments and git diff checks pass. Native fixture hashes are unchanged.

## Documentation assurance

DQ-5611-SET-FWEEK-CONVERSION-001 requires documented finite/domain admission,
bare literal range-failure rejection and preserved malformed/quoted controls,
negative-only compatibility, type/evaluation limits, serial reproduction,
state/event failure atomicity, downstream setting use and rollback.
DV-5611-SET-FWEEK-CONVERSION-001: maintainer-authorized agent self-review
on 2026-10-06 checked source/output identity, independent expectations,
rejection before mutation/event, four catalogs, walkthrough and completed
automated evidence above. This is not independent-human review.
The review-fix walkthrough additionally checks bare 1E999 and 1E-999 reject
with 46 while retaining FWEEK 2, WEEK result 53 and no success event in both
modes; quoted out-of-range/malformed controls still select fallback 1.

Procedural delta: unchecked Numeric round/clamp is replaced for FWEEK only
by checked truncation/domain rejection; explicit VFP9 keeps negative aliases.
Omission and ordinary other coercions/evaluation fallback remain unchanged.
Misuse severity is medium: assuming an invalid operand silently selects a
different first-week policy, or claiming full type/week parity, can mislead calendar results.
HZ-runtime-crash-01 and HZ-data-corruption-01 link defined conversion and no
partial setting/event publication.

Walkthrough: each independent case starts at FWEEK 2. Default 1.9 selects 1;
0, 4 or 1E300 raises 46 and preserves 2. Explicit VFP9 -4294967295 selects 1;
positive 4294967297 still raises 46 without a success event. Catch the error,
read SET('FWEEK'), and reset explicitly rather than assuming a clamp.
Fresh tests verify WEEK({^2021-01-01},0,1) consumes the retained/selected setting
(1/53/52 for FWEEK 1/2/3), then omitted reset returns to 1 and mode resets to
COPPERFIN.

Rollback: revert or disable the bounded production change through version
control, retaining fixture/regression patch as evidence. Restore
SET NUMERICBEHAVIOR TO COPPERFIN and SET FWEEK TO 1. A diagnostic rollback
with helpers/tests retained but original FWEEK dispatch restored reproduces
the old-path assertion failures above; they expose the safety/compatibility
regression rather than define the requirement. No persistent data/system
repair is required by the probe.
Field-notification plan: if released evidence/guidance is wrong, notify
affected users/maintainers with exact revision/fixture identity, mode and
known limits; correct requirements/docs/tests together and revalidate.
