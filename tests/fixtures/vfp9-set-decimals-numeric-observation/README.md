# Installed VFP9 SET DECIMALS numeric observations

Recovered 2026-10-06 from installed Visual FoxPro 09.00.0000.7423 under Wine
using the unchanged owner-installed /home/rich/bin/vfp9-probe runner.
decimals.prg is complete clean-room source; decimals.out retains VERSION,
50 evaluated operands, omitted reset and final setting in 53 lines.
Three fresh processes produced matching output; the last two were compared
byte-for-byte. Each attempted setting starts from DECIMALS 5; cleanup restores
DECIMALS 2. No table, SQL/backend/network connection, VM, installer or system
change was used. Generated FXP is ignored; no proprietary binary was inspected.

## Selected requirement and boundary

RQ-CF-PRG-SET-DECIMALS-NUMERIC-001 derives from owner-approved #5611/#6776,
RQ-CF-PRG-NUMERIC-BEHAVIOR-001 and HZ-runtime-crash-01/HZ-data-corruption-01.
Only SET DECIMALS TO's integer-setting dispatch is selected. Numeric operands
truncate toward zero into 0..18 (18.9 -> 18, -0.9 -> 0). Default COPPERFIN
rejects non-finite or out-of-domain operands before conversion. Explicit VFP9
preserves both-sign signed-low-32 aliases and huge/infinite integer-indefinite
zero before the same domain. Rejection raises localized catchable error 10
before setting mutation or a successful SET event. An omitted operand resets
to 2 in both modes.

The 43 Numeric native rows independently expose the actual converted setting,
including fractional truncation, 0/18 admission, invalid converted values,
both-sign aliases and indefinite zero, with error 10 preserving DECIMALS 5.
Exact int64/uint64 values keep their low bits without a double round trip;
NaN rejects in default mode and takes shared-model indefinite zero in VFP9.
Those exact-integer/NaN policies derive from the parent checked-conversion
requirement and hazard, not native extended-integer/NaN observations.
The operation-specific wrapper shares only the existing checked signed-int32
conversion model used by AFONT, not any font semantics.

Bare Numeric tokens must enter the Numeric conversion domain rather than the
old Character coercion path; evaluated parenthesized operands preserve their
actual kind. Existing expression-evaluation fallback still resets to 2; this
slice does not redesign SET evaluation/resumption. Non-Numeric operands retain
ordinary half-away rounding/clamping or fallback to 2 with checked conversion;
these are preserved controls, not recovered native type parity. Native Currency,
Logical, NULL and Character controls all raise 10 and preserve 5, unlike
existing Copperfin coercions. That separate gap is #7006, reported against
exact main d5b1031df8d010c2534a8cece03a193de937d7d3 without implementation
admission. FDOW/FWEEK/EPOCH, other settings and formatting behavior stay separate.
There is no argument-sized allocation, loop or new production backend.

## Reproduction and identity

From the repository root:

```sh
/home/rich/bin/vfp9-probe tests/fixtures/vfp9-set-decimals-numeric-observation/decimals.prg
sha256sum tests/fixtures/vfp9-set-decimals-numeric-observation/decimals.prg tests/fixtures/vfp9-set-decimals-numeric-observation/decimals.out
```

Source SHA-256: a5ae30980cf75754aa7c36d7b75b1c022c54159aa64985a8bc4a1e3201ece2f6.
Output SHA-256: 9c90bf64ddb602ec6ed568b0261d99e53bbe86432fa6207fe9a94a613eb46025.
VR-5611-SET-DECIMALS-NATIVE-001: three fresh runs match all 53 lines;
the last two match byte-for-byte. Error 10 preserves 5; omitted reset gives 2.

## Focused verification

146 direct calls cover an independent Numeric table, adjacent domain/wrap
doubles, exact extended integers, NaN and preserved ordinary coercions.
188 fresh runtime cases cover both modes and bare/parenthesized Numeric tokens,
catchability, exact setting/preservation, omitted reset, mode reset and successful
event suppression. Four additional cases check translated diagnostics/code/state
in every shipped locale, scoped/restored through COPPERFIN_LOCALE.
Runtime numeric tests must run serially across build directories because this
suite and its neighboring numeric cases own common temporary paths.

```sh
cmake -S . -B /home/rich/temp/copperfin-set-decimals-5611-build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS_DEBUG='-O0 -g1'
cmake --build /home/rich/temp/copperfin-set-decimals-5611-build --target test_prg_engine_numeric_behavior test_prg_engine_relations test_prg_engine_database_lifecycle test_prg_engine_date_time_functions test_localization -j4
ctest --test-dir /home/rich/temp/copperfin-set-decimals-5611-build -R '^test_prg_engine_(numeric_behavior|relations|database_lifecycle|date_time_functions)$' --output-on-failure
cmake -S . -B /home/rich/temp/copperfin-set-decimals-5611-sanitize -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_CXX_FLAGS_DEBUG='-O0 -g1 -fsanitize=address,undefined,float-cast-overflow -fno-omit-frame-pointer' -DCMAKE_EXE_LINKER_FLAGS='-fsanitize=address,undefined,float-cast-overflow'
cmake --build /home/rich/temp/copperfin-set-decimals-5611-sanitize --target test_prg_engine_numeric_behavior test_prg_engine_relations test_prg_engine_database_lifecycle test_prg_engine_date_time_functions -j2
ctest --test-dir /home/rich/temp/copperfin-set-decimals-5611-sanitize -R '^test_prg_engine_(numeric_behavior|relations|database_lifecycle|date_time_functions)$' --output-on-failure
ctest --test-dir /home/rich/temp/copperfin-set-decimals-5611-build -R '^test_(locale_catalog_install_contract|native_test_isolation_contract|localization|changelog_fragment_assembler|changelog_fragments_valid|contributor_signoff_contract|agent_channel_contract)$' --output-on-failure
```

VR-5611-SET-DECIMALS-REGRESSION-001: corrected original dispatch fails exactly
264 selected conversion/state/event/locale assertions in GCC (numeric 7.65s;
full four-suite run 9.06s) and Clang sanitizers (numeric 32.64s; full 38.86s).
All direct helper checks, preserved coercion controls, existing numeric rows and
three neighbors pass before dispatch changes; no sanitizer diagnostic occurred.
An initial helper prototype omitted invalid Character coercion's prior reset-2
fallback and aborted; that run is explicitly excluded from retained baseline
evidence. The fallback was corrected before the two matching 264-failure runs.

Final GCC passes 4/4 in 8.85s (numeric 7.40s). Catalog/install, localization,
test-isolation, contributor-signoff, changelog and live-channel contracts pass
7/7 in 21.05s; all 146 fragments validate.
VR-5611-SET-DECIMALS-SANITIZER-001: final Clang ASan/UBSan/float-cast-overflow
passes 4/4 in 38.24s (numeric 32.46s) without sanitizer diagnostics.

Fixture identity and git diff --check pass. Local evidence is not independent-
human, installed-GUI, complete SET evaluation or full formatting parity evidence.
Hosted exact-head checks/review remain separate integration gates.

## Documentation assurance

DQ-5611-SET-DECIMALS-CONVERSION-001 requires documented finite/domain admission,
explicit compatibility aliases, type/evaluation limits, serial reproduction,
state/event atomicity and rollback.
DV-5611-SET-DECIMALS-CONVERSION-001: maintainer-authorized agent self-review
on 2026-10-06 checked source/output identity, independent expectations, rejection
before mutation/event, all catalogs, procedural walkthrough and completed
automated evidence above. This is not independent-human review.

Procedural delta: the old unchecked round/clamp path is replaced for Numeric
SET DECIMALS only by checked truncation/domain rejection; explicit VFP9 keeps
observed aliases; omission and ordinary other coercion/evaluation fallbacks stay.
Misuse severity is medium: treating invalid input as silently clamped, or
confusing conversion with type/formatting/evaluation parity, can mislead numeric
display or operator expectations. HZ-runtime-crash-01 and HZ-data-corruption-01
link defined conversion and no partial setting/event publication.

Walkthrough: each independent case starts at DECIMALS 5. Default 1.9 selects
1; 19 or 1E300 raises 10 while 5 remains unchanged. Explicit VFP9 4294967297
selects 1; an invalid
converted value still raises 10 without a successful event. Catch the error,
read SET('DECIMALS') and reset explicitly rather than assuming a clamp.
The fresh tests verify this sequence plus omitted reset and mode cleanup.

Rollback: revert or disable this bounded production change through version
control, preserving the complete fixture/regression patch as evidence. Restore
SET NUMERICBEHAVIOR TO COPPERFIN and SET DECIMALS TO 2. A diagnostic rollback
run retaining the checked helpers/tests but restoring only the original
DECIMALS dispatch reproduces the 264 old-path failures; they expose the
compatibility/safety regression rather than becoming a new requirement.
No persistent product data or system repair is required by this probe.
Field-notification plan: if released evidence or guidance is wrong, notify
affected users/maintainers with exact revision/fixture identity, mode and known
limits; correct requirement/docs/tests together and revalidate before retagging.
