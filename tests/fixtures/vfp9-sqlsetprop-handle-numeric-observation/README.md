# Installed VFP9 SQLSETPROP handle observations

Recovered 2026-10-06 from installed Visual FoxPro 09.00.0000.7423 under Wine
using the unchanged owner-installed /home/rich/bin/vfp9-probe runner.
sqlsetprop.prg is the complete clean-room source; sqlsetprop.out retains
VERSION, 48 calls, the unchanged Asynchronous default before/after, and
unchanged data-session 1. Three fresh connection-free processes produced
byte-identical 52-line output; the third was compared directly with the
retained output using cmp. All setters use Logical .F. as their third
argument, so separate property-value Numeric conversion is not selected.
No SQL connection, ODBC driver/backend, network, persistent table, VM,
installer or system change was used. Default-setting calls affect only the
fresh probe process and leave the observed default unchanged. Generated FXP
is ignored; no proprietary binary was inspected.

## Selected requirement and boundary

RQ-CF-PRG-SQLSETPROP-HANDLE-NUMERIC-001 derives from owner-approved #5611/#6776,
RQ-CF-PRG-NUMERIC-BEHAVIOR-001 and HZ-runtime-crash-01/HZ-data-corruption-01.
Only SQLSETPROP's first-argument llround/int expression-dispatch site is
selected. Numeric fractions truncate toward zero. Default COPPERFIN requires
checked finite signed-int32 conversion. Explicit VFP9 additionally preserves
negative Numeric signed-low-32 aliases and huge/infinite zero; positive
operands never use those aliases. Rejection raises localized catchable 1466
before the property setter callback. Exact int64/uint64 avoid double rounding;
NaN rejects both modes as derived safety policy, not a native NaN claim.
Existing other coercions retain checked half-away rounding without native
type-parity claims. The operation-specific helper shares only the independently
observed identical SQLGETPROP handle conversion.

The 41 Numeric native calls independently distinguish near-zero truncation,
positive huge rejection versus negative zero aliases, and error 1466.
All absent nonzero handles produce 1466, so their precise converted indices
are not independently distinguished: those indices use documented shared
checked/truncating and negative-low-32 policy.
PRG rows reuse the existing synthetic in-process connection (handle 1), reset
its Boolean Asynchronous property before each call, and compare return status
plus property state against independent table expectations. Ordinary 1.5/1.9
and negative handle-1 aliases therefore expose incorrect setter mutations.
This is not native connected-backend, default-setter or invalid-result parity.

Other SQL callers, SQLSETPROP's separate property-value conversions,
connection/backend/session behavior, zero-handle defaults, invalid setter
results and non-Numeric admission remain outside scope.
Native zero-handle setters return Numeric 1, absent nonzero handles raise
1466 and Currency/Logical/NULL/Character controls raise 11. Existing
callback/coercion gaps are preserved and reported separately as #6999 against exact
main e3bdb6458f6b1c5f801f530c29d06ee8fff6bf29, without implementation admission.
The general shared Numeric formatter gap #6997 is unchanged: only this
rejection diagnostic uses the existing safe round-trip decimal formatter.
Finite huge and infinite caught messages retain the original operand as
derived diagnostic safety policy, not native error-text parity.
No argument-sized allocation/loop or connection creation is added.

## Reproduction and identity

From the repository root:

```sh
/home/rich/bin/vfp9-probe tests/fixtures/vfp9-sqlsetprop-handle-numeric-observation/sqlsetprop.prg
sha256sum tests/fixtures/vfp9-sqlsetprop-handle-numeric-observation/sqlsetprop.prg tests/fixtures/vfp9-sqlsetprop-handle-numeric-observation/sqlsetprop.out
```

Source SHA-256: 34b0ebe4990c0dcad0cc10a619f286e4a5656f492ea551e616a10a15097eca45.
Output SHA-256: 08781bd61a745f3d81b9c505e84066c8e35b3179b3d86515bb85e4fa667faeea.
VR-5611-SQLSETPROP-NATIVE-001: three fresh runs match all 52 output lines,
including 48 setters, unchanged default before/after and data-session 1.

## Focused verification

134 direct calls cover the independent Numeric table, adjacent signed-int32
doubles, exact extended integers, NaN and preserved checked coercions.
118 PRG rows cover return/mutation state, catchability with unchanged
connection/action/property on rejection, safe caught diagnostics, preserved
setter/coercion controls, mode reset and post-disconnect absence. No external
SQL connection is opened. Runtime ctest commands must run serially across
directories because the numeric suite owns a common temporary fixture path:

```sh
cmake -S . -B /home/rich/temp/copperfin-sqlsetprop-handle-5611-build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS_DEBUG='-O0 -g1'
cmake --build /home/rich/temp/copperfin-sqlsetprop-handle-5611-build --target test_prg_engine_numeric_behavior test_prg_engine_sql_cursors_metadata test_prg_engine_arrays test_prg_engine_string_math_functions -j4
ctest --test-dir /home/rich/temp/copperfin-sqlsetprop-handle-5611-build -R '^test_prg_engine_(numeric_behavior|sql_cursors_metadata|arrays|string_math_functions)$' --output-on-failure
cmake -S . -B /home/rich/temp/copperfin-sqlsetprop-handle-5611-sanitize -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_CXX_FLAGS_DEBUG='-O0 -g1 -fsanitize=address,undefined,float-cast-overflow -fno-omit-frame-pointer' -DCMAKE_EXE_LINKER_FLAGS='-fsanitize=address,undefined,float-cast-overflow'
cmake --build /home/rich/temp/copperfin-sqlsetprop-handle-5611-sanitize --target test_prg_engine_numeric_behavior test_prg_engine_sql_cursors_metadata test_prg_engine_arrays test_prg_engine_string_math_functions -j4
ctest --test-dir /home/rich/temp/copperfin-sqlsetprop-handle-5611-sanitize -R '^test_prg_engine_(numeric_behavior|sql_cursors_metadata|arrays|string_math_functions)$' --output-on-failure
ctest --test-dir /home/rich/temp/copperfin-sqlsetprop-handle-5611-build -R '^test_(changelog_fragment_assembler|changelog_fragments_valid|contributor_signoff_contract|agent_channel_contract)$' --output-on-failure
```

VR-5611-SQLSETPROP-REGRESSION-001: original expression dispatch fails exactly
47 new conversion/caught-message assertions in GCC (numeric 6.49s; full
four-suite run 8.27s) and Clang sanitizers (numeric 29.68s; full run 36.72s).
Both operation-specific direct helpers, preserved setter/coercion controls,
the existing SQLGETPROP rows and all three neighbors pass before dispatch
changes. No baseline sanitizer diagnostic was produced.
After changing only SQLSETPROP's first-argument dispatch and using the existing
safe Numeric diagnostic formatter, GCC passes 4/4 in 7.64s (numeric 5.93s).
Changelog assembly/fragment, contributor-signoff and live-channel contracts
pass 4/4 in 5.65s; all 143 fragments validate.
VR-5611-SQLSETPROP-SANITIZER-001: final Clang ASan/UBSan/float-cast-overflow
passes 4/4 in 34.64s (numeric 27.80s) without sanitizer diagnostics.
Runtime tests ran serially across build directories. Fixture identities and
git diff --check pass. Hosted exact-head checks/review remain separate
integration gates. Local evidence is not independent-human, installed-GUI,
native connected SQL or full SQLSETPROP/product parity evidence.

## Documentation assurance

DQ-5611-SQLSETPROP-CONVERSION-001 requires documented admission, negative-only
compatibility aliases, setter/property-value boundaries, serial reproduction
and rollback.
DV-5611-SQLSETPROP-CONVERSION-001: maintainer-authorized agent self-review
verified fixture identities, independent expectations, error-before-setter
walkthrough and completed automated results above before shipping.
No independent-human verification is claimed.

Procedural delta: native reproduction is connection-free and changes only
the fresh process's already-false default; local synthetic normal/sanitizer
tests use only Boolean property values, reset the property before each call,
and cover conversion plus three neighbors without ODBC/network, VM, installer
or system changes.
Misuse severity is medium under HZ-runtime-crash-01/HZ-data-corruption-01:
do not infer repaired backend/default-setter/type admission, other SQL callers
or property-value conversion safety from these state comparisons.
Walkthrough outcome: rejected conversion preserves the synthetic connection,
Boolean property and last-action metadata; admitted handle-1 setters mutate
the property, reset restores it, and disconnect cleans the connection.
Rollback reverts only this conversion while retaining independent unchecked-
path regressions. Incorrect guidance requires correcting the matrix/handoff/
release-PR evidence and notifying the owner before reuse.
