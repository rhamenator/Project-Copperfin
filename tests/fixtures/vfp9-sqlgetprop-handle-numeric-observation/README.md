# Installed VFP9 SQLGETPROP handle observations

Recovered 2026-10-06 from installed Visual FoxPro 09.00.0000.7423 under Wine
using the unchanged owner-installed /home/rich/bin/vfp9-probe runner.
sqlgetprop.prg is the complete clean-room source; sqlgetprop.out retains
VERSION, 48 calls and unchanged data-session 1. Two fresh connection-free
processes produced byte-identical 50-line output. No SQL connection, ODBC
driver/backend, network, persistent table, VM, installer or system change
was used. Generated FXP is ignored; no proprietary binary was inspected.

## Selected requirement and boundary

RQ-CF-PRG-SQLGETPROP-HANDLE-NUMERIC-001 derives from #5611/#6776,
RQ-CF-PRG-NUMERIC-BEHAVIOR-001 and HZ-runtime-crash-01/HZ-data-corruption-01.
Only SQLGETPROP's first-argument llround/int site is selected.
Numeric fractions truncate toward zero. Default COPPERFIN requires checked
finite signed-int32 conversion. Explicit VFP9 additionally preserves negative
Numeric signed-low-32 aliases and huge/infinite zero; positive values never
use those aliases. Rejection raises localized catchable error 1466 before
the property query callback. Exact int64/uint64 avoid double rounding;
NaN rejects both modes as derived safety policy, not a native NaN claim.
Other existing coercions retain checked half-away rounding without native
type-parity claims.

The 41 Numeric native calls independently distinguish near-zero truncation,
positive huge rejection versus negative zero aliases, and error 1466.
Absent nonzero handles all produce 1466, so their precise converted indices
are not independently distinguished: those indices use documented shared
checked/truncating and negative-low-32 policy.
Canonical PRG comparisons use the existing synthetic in-process connection
(handle 1) to distinguish ordinary 1.5/1.9 and negative aliases. This does not
claim native connected-backend, default-property or invalid-query result parity.

SQL property values (including SQLSETPROP's separate conversions), other SQL
callers, connections, backends, session state, zero-handle defaults, invalid
query results and non-Numeric admission are not changed.
Native zero-handle queries return Logical false, invalid converted handles
raise 1466 and Currency/Logical/NULL/Character controls raise 11. Existing
callback/coercion gaps are preserved and tracked separately in #6996 against
main 369d5f41bb162c909c497fc097c982957289d574, without implementation admission.
Review of this rejection path also exposed the shared Numeric text formatter's
out-of-int64 llround (#6997). Only this diagnostic uses the existing safe
round-trip decimal formatter; the shared formatter and its other callers are
not changed. Finite huge and infinite caught messages retain the original
operand as derived diagnostic safety policy, not native error-text parity.
No argument-sized allocation, query loop,
connection creation or mutation is added to production.

## Reproduction and identity

From the repository root:

```sh
/home/rich/bin/vfp9-probe tests/fixtures/vfp9-sqlgetprop-handle-numeric-observation/sqlgetprop.prg
sha256sum tests/fixtures/vfp9-sqlgetprop-handle-numeric-observation/sqlgetprop.prg tests/fixtures/vfp9-sqlgetprop-handle-numeric-observation/sqlgetprop.out
```

Source SHA-256: e7844ecab1ac4c37c29b78df364de9ca828226d01f06e52f89fdb048edbd4b35.
Output SHA-256: 94095c99d34e26a35033b020ca91215f72893ea1c736ef89fd7f5aa207dbc412.
VR-5611-SQLGETPROP-NATIVE-001: two fresh runs match all 50 output lines,
including 48 queries and unchanged data-session 1.

## Focused verification

134 direct calls cover the independent Numeric table, adjacent signed-int32
doubles, exact extended integers, NaN and preserved checked coercions.
119 PRG rows cover conversion, a live synthetic handle, catchability with
unchanged connection/action, preserved query/coercion controls, mode reset,
disconnect and subsequent absence. No external SQL connection is opened.
Run runtime ctest commands serially across directories because the numeric
suite owns a common temporary fixture path:

```sh
cmake -S . -B /home/rich/temp/copperfin-sqlgetprop-handle-5611-build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS_DEBUG='-O0 -g1'
cmake --build /home/rich/temp/copperfin-sqlgetprop-handle-5611-build --target test_prg_engine_numeric_behavior test_prg_engine_sql_cursors_metadata test_prg_engine_arrays test_prg_engine_string_math_functions -j4
ctest --test-dir /home/rich/temp/copperfin-sqlgetprop-handle-5611-build -R '^test_prg_engine_(numeric_behavior|sql_cursors_metadata|arrays|string_math_functions)$' --output-on-failure
cmake -S . -B /home/rich/temp/copperfin-sqlgetprop-handle-5611-sanitize -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_CXX_FLAGS_DEBUG='-O0 -g1 -fsanitize=address,undefined,float-cast-overflow -fno-omit-frame-pointer' -DCMAKE_EXE_LINKER_FLAGS='-fsanitize=address,undefined,float-cast-overflow'
cmake --build /home/rich/temp/copperfin-sqlgetprop-handle-5611-sanitize --target test_prg_engine_numeric_behavior test_prg_engine_sql_cursors_metadata test_prg_engine_arrays test_prg_engine_string_math_functions -j4
ctest --test-dir /home/rich/temp/copperfin-sqlgetprop-handle-5611-sanitize -R '^test_prg_engine_(numeric_behavior|sql_cursors_metadata|arrays|string_math_functions)$' --output-on-failure
```

VR-5611-SQLGETPROP-REGRESSION-001: original expression dispatch fails exactly
43 conversion assertions with the initial 115 PRG rows in GCC (numeric 6.13s;
full four-suite run 7.80s) and
Clang sanitizers (numeric 27.68s; full run 34.34s). The new direct helper
boundaries and preserved query/coercion controls pass before dispatch changes;
all three neighbors pass. No sanitizer diagnostic was produced in this baseline.
After changing only SQLGETPROP dispatch, GCC passes 4/4 in 7.63s (numeric 5.94s).
Four additional caught-message rows expose the separate diagnostic defect:
two finite-huge assertions fail in 5.86s (integer-indefinite text instead of
original 1E300), while both infinite-message controls pass. The selected
diagnostic now uses the safe formatter; #6997 retains the general gap.
With all 119 PRG rows and the safe diagnostic, GCC passes 4/4 in 7.83s
(numeric 6.11s). Changelog assembly/fragment, live-channel and full safety-
traceability workflow contracts pass 4/4 in 360.90s. Final fragment checks
additionally pass 2/2 in 0.45s; all 142 fragments validate.
VR-5611-SQLGETPROP-SANITIZER-001: final Clang ASan/UBSan/float-cast-overflow
passes 4/4 in 34.19s (numeric 27.45s), including all 119 PRG rows and safe
diagnostics, without sanitizer diagnostics. The initial conversion-only run
passed 4/4 in 34.59s (numeric 27.33s). Tests ran
serially across build directories. Fixture identities and git diff --check pass.
Hosted exact-head checks and review are separate integration gates.
Local evidence is not independent-human, native connected SQL, installed-GUI
or full SQLGETPROP/product parity evidence.

## Documentation assurance

DQ-5611-SQLGETPROP-CONVERSION-001 requires documented admission, negative-only
compatibility aliases, query boundaries, serial reproduction and rollback.
DV-5611-SQLGETPROP-CONVERSION-001: maintainer-authorized agent self-review
verified fixture identities, independent expectations, error-before-callback
walkthrough and completed automated results above before shipping.
No independent-human verification is claimed.

Procedural delta: native reproduction is connection-free; local synthetic
normal/sanitizer tests cover this conversion and three neighboring suites
without an ODBC/network connection, VM, installer or system change.
Misuse severity is medium under HZ-runtime-crash-01/HZ-data-corruption-01:
do not infer repaired backend/default-query/type admission, other SQL callers
or property-value safety from these canonical comparisons.
Walkthrough outcome: rejected conversion preserves the synthetic connection
and last-action metadata; admitted queries continue and disconnect cleans it.
Rollback reverts only this conversion while retaining independent unchecked-
path regressions. Incorrect guidance requires correcting the matrix/handoff/
release-PR evidence and notifying the owner before reuse.
