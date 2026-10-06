# Installed VFP9 SET FDOW numeric observations

Recovered 2026-10-06 from installed Visual FoxPro 09.00.0000.7423 under Wine
using the unchanged owner-installed /home/rich/bin/vfp9-probe runner.
fdow.prg is complete clean-room source; fdow.out retains VERSION, 50 evaluated
operands, omitted reset and final setting in 53 lines. Three fresh processes
match; all three final outputs were compared byte-for-byte. The omitted
setting readback uses ALLTRIM only to remove display padding. Each attempt begins with
FDOW 5. Cleanup restores FDOW 1. No table, SQL/network/backend, VM, installer
or system change was used. Generated FXP is ignored; no proprietary binary
was inspected.

## Selected requirement and boundary

RQ-CF-PRG-SET-FDOW-NUMERIC-001 derives from owner-approved #5611/#6776,
RQ-CF-PRG-NUMERIC-BEHAVIOR-001 and HZ-runtime-crash-01/HZ-data-corruption-01.
Only SET FDOW TO's integer-setting dispatch is selected. Numeric operands
truncate toward zero into 1..7 (1.9 -> 1, 7.9 -> 7). Default COPPERFIN
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

Bare Numeric tokens enter Numeric conversion, not Character coercion;
parenthesized operands retain their actual kind. Existing evaluation exceptions
still fall back to 1; evaluation/resumption is not redesigned. Other ordinary
Currency/logical/null/Character operands retain checked half-away rounding,
clamping/fallback to 1, not native type parity. Native controls instead raise
10 while retaining 5; separate gap #7008 against exact main
980b2c3aa97483af4db2c69b20c0b30c5956441a is not admitted into this slice.
FWEEK/EPOCH, other settings and date/week algorithms remain separate.
There is no argument-sized allocation, loop or new backend.

## Reproduction and identity

From repository root:

```sh
/home/rich/bin/vfp9-probe tests/fixtures/vfp9-set-fdow-numeric-observation/fdow.prg
sha256sum tests/fixtures/vfp9-set-fdow-numeric-observation/fdow.prg tests/fixtures/vfp9-set-fdow-numeric-observation/fdow.out
```

Source SHA-256: dc4c0a36bf3ab596256ff0583326be6b8734b1f7e9619d8976aea9bbb84de8a5.
Output SHA-256: e84a4451b6779a86c161df2b3cb9784f6a74e4e49affeab5e476ab034232b95d.
VR-5611-SET-FDOW-NATIVE-001: three fresh outputs match; last two byte-for-byte.
Error 46 preserves 5; negative aliases select 1/7; omitted reset gives 1.

## Focused verification

144 direct calls cover 43 native Numeric and 29 adjacent/extended/NaN/coercion
cases in both modes. 188 fresh runtime cases cover bare and parenthesized
Numeric tokens, catchability, exact setting/preservation, DOW's use of FDOW,
omitted reset, mode reset and success-event suppression. Four additional cases
check diagnostic text/code/state in all shipped locales with scoped/restored
COPPERFIN_LOCALE. Runtime numeric tests run serially across build directories
because this suite and neighboring numeric tests own shared temporary paths.

```sh
cmake -S . -B /home/rich/temp/copperfin-set-fdow-5611-build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS_DEBUG='-O0 -g1'
cmake --build /home/rich/temp/copperfin-set-fdow-5611-build --target test_prg_engine_numeric_behavior test_prg_engine_relations test_prg_engine_database_lifecycle test_prg_engine_date_time_functions test_localization -j4
ctest --test-dir /home/rich/temp/copperfin-set-fdow-5611-build -R '^test_prg_engine_(numeric_behavior|relations|database_lifecycle|date_time_functions)$' --output-on-failure
cmake -S . -B /home/rich/temp/copperfin-set-fdow-5611-sanitize -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_CXX_FLAGS_DEBUG='-O0 -g1 -fsanitize=address,undefined,float-cast-overflow -fno-omit-frame-pointer' -DCMAKE_EXE_LINKER_FLAGS='-fsanitize=address,undefined,float-cast-overflow'
cmake --build /home/rich/temp/copperfin-set-fdow-5611-sanitize --target test_prg_engine_numeric_behavior test_prg_engine_relations test_prg_engine_database_lifecycle test_prg_engine_date_time_functions -j2
ctest --test-dir /home/rich/temp/copperfin-set-fdow-5611-sanitize -R '^test_prg_engine_(numeric_behavior|relations|database_lifecycle|date_time_functions)$' --output-on-failure
ctest --test-dir /home/rich/temp/copperfin-set-fdow-5611-build -R '^test_(locale_catalog_install_contract|native_test_isolation_contract|localization|changelog_fragment_assembler|changelog_fragments_valid|contributor_signoff_contract|agent_channel_contract)$' --output-on-failure
```

VR-5611-SET-FDOW-REGRESSION-001: original FDOW dispatch fails exactly
572 selected setting/error/event/locale/DOW-consumer assertions in both GCC
(numeric 8.15s; four-suite run 9.57s) and Clang sanitizers (numeric 36.35s;
full run 42.64s). All helper checks, preserved coercions and existing Numeric
rows plus three neighbors pass before dispatch changes. No baseline sanitizer
diagnostic occurred.
Final GCC passes 4/4 in 10.27s (numeric 8.93s).
VR-5611-SET-FDOW-SANITIZER-001: final Clang ASan/UBSan/float-cast-overflow
passes 4/4 in 43.18s (numeric 36.99s) without diagnostics.
Catalog/install, localization/isolation, signoff, changelog and live-channel
contracts pass 7/7 in 18.47s; all 147 fragments validate.
Fixture identity and git diff --check pass. Local evidence is not independent-
human, installed-GUI, complete SET evaluation or full date/week parity evidence.
Hosted exact-head checks/review remain separate integration gates.

## Documentation assurance

DQ-5611-SET-FDOW-CONVERSION-001 requires documented finite/domain admission,
negative-only compatibility, type/evaluation limits, serial reproduction,
state/event failure atomicity, downstream setting use and rollback.
DV-5611-SET-FDOW-CONVERSION-001: maintainer-authorized agent self-review
on 2026-10-06 checked source/output identity, independent expectations,
rejection before mutation/event, four catalogs, walkthrough and completed
automated evidence above. This is not independent-human review.

Procedural delta: unchecked Numeric round/clamp is replaced for FDOW only
by checked truncation/domain rejection; explicit VFP9 keeps negative aliases.
Omission and ordinary other coercions/evaluation fallback remain unchanged.
Misuse severity is medium: assuming an invalid operand silently selects a
different day, or claiming full type/date parity, can mislead calendar results.
HZ-runtime-crash-01 and HZ-data-corruption-01 link defined conversion and no
partial setting/event publication.

Walkthrough: each independent case starts at FDOW 5. Default 1.9 selects 1;
0, 8 or 1E300 raises 46 and preserves 5. Explicit VFP9 -4294967295 selects 1;
positive 4294967297 still raises 46 without a success event. Catch the error,
read SET('FDOW'), and reset explicitly rather than assuming a clamp.
Fresh tests verify DOW({^2026-10-04},0) consumes the retained/selected setting,
then omitted reset returns to 1 and mode resets to COPPERFIN.

Rollback: revert or disable the bounded production change through version
control, retaining fixture/regression patch as evidence. Restore
SET NUMERICBEHAVIOR TO COPPERFIN and SET FDOW TO 1. A diagnostic rollback
with helpers/tests retained but original FDOW dispatch restored reproduces
the old-path assertion failures above; they expose the safety/compatibility
regression rather than define the requirement. No persistent data/system
repair is required by the probe.
Field-notification plan: if released evidence/guidance is wrong, notify
affected users/maintainers with exact revision/fixture identity, mode and
known limits; correct requirements/docs/tests together and revalidate.
