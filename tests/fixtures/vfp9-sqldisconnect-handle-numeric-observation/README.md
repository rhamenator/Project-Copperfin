# Installed VFP9 SQLDISCONNECT handle observations

Recovered 2026-10-06 from installed Visual FoxPro 09.00.0000.7423 under Wine
using the unchanged owner-installed /home/rich/bin/vfp9-probe runner.
sqldisconnect.prg is the complete clean-room source; sqldisconnect.out retains
VERSION, 48 calls and unchanged data-session 1. Three fresh connection-free
processes produced matching 50-line output; the last two were compared
byte-for-byte. No SQL connection, ODBC driver/backend, network, persistent
table, VM, installer or system change was used. Disconnect-zero calls affect
only the already-connection-free probe process. Generated FXP is ignored;
no proprietary binary was inspected.

## Selected requirement and boundary

RQ-CF-PRG-SQLDISCONNECT-HANDLE-NUMERIC-001 derives from owner-approved #5611/#6776,
RQ-CF-PRG-NUMERIC-BEHAVIOR-001 and HZ-runtime-crash-01/HZ-data-corruption-01.
Only SQLDISCONNECT's first-argument llround/int expression-dispatch site is
selected. Numeric fractions truncate toward zero. Default COPPERFIN requires
checked finite signed-int32 conversion. Explicit VFP9 additionally preserves
negative Numeric signed-low-32 aliases and huge/infinite zero; positive
operands never use those aliases. Rejection raises localized catchable 1466
before the disconnect callback and successful-disconnect event. Exact
int64/uint64 avoid double rounding; NaN rejects both modes as derived safety
policy, not a native NaN claim. Other existing coercions retain checked
half-away rounding without native type-parity claims. The operation-specific
helper shares only the independently observed identical SQLGETPROP conversion.

The 41 Numeric native calls distinguish near-zero truncation, positive huge
rejection versus negative zero aliases, and error 1466. All absent nonzero
handles produce 1466, so their precise converted indices are not independently
distinguished: those indices use documented shared checked/truncating and
negative-low-32 policy.

Every PRG case starts with a fresh synthetic in-process connection at handle 1.
Independent expectations check return/error, connection presence and last-action
metadata, then reset the mode, disconnect any surviving handle and verify its
absence. Ordinary 1.5/1.9 and negative handle-1 aliases expose unintended or
missing removal. Rejection leaves handle 1 and its connect action unchanged.
These tests exercise conversion, not native connected-backend/lifecycle parity.

Disconnect-all, absent-handle results, non-Numeric admission, session/backend
lifecycle and other SQL callers remain outside scope. Native zero returns
Numeric 1, absent nonzero handles raise 1466, and Currency/Logical/NULL/Character
controls raise 11. Existing callback/coercion gaps are preserved and reported
separately as #7001 against exact main
4aef3ae3052a3d004da5dbe48bbbb8189a810cd1, without implementation admission.
The general shared Numeric formatter gap #6997 is unchanged; only this
rejection diagnostic uses the existing safe round-trip decimal formatter.
Finite huge/infinite caught messages retain the original operand as derived
diagnostic safety policy, not native error-text parity.
No argument-sized allocation/loop or new production connection is added.

## Reproduction and identity

From the repository root:

```sh
/home/rich/bin/vfp9-probe tests/fixtures/vfp9-sqldisconnect-handle-numeric-observation/sqldisconnect.prg
sha256sum tests/fixtures/vfp9-sqldisconnect-handle-numeric-observation/sqldisconnect.prg tests/fixtures/vfp9-sqldisconnect-handle-numeric-observation/sqldisconnect.out
```

Source SHA-256: 701a19d1bf4f2576d186c003b59ae84f3902635026bac578dd58f4e0c485e508.
Output SHA-256: 484f8d8a02cf2e839c312fdd2309204569e8d3f0ddfc0f7092838c94791a1545.
VR-5611-SQLDISCONNECT-NATIVE-001: three fresh runs match all 50 output lines,
including 48 disconnect calls and unchanged data-session 1.

## Focused verification

134 direct calls cover the independent Numeric table, adjacent signed-int32
doubles, exact extended integers, NaN and preserved checked coercions.
300 PRG rows (100 fresh-session cases, three rows each) cover both modes,
connection removal/preservation, catchability, last-action metadata, safe
caught diagnostics, preserved callback/coercion controls, mode reset and
cleanup. No external SQL connection is opened. Runtime ctest commands must
run serially across directories: the existing numeric suite owns a common
temporary path, as does the new disconnect suite.

```sh
cmake -S . -B /home/rich/temp/copperfin-sqldisconnect-handle-5611-build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS_DEBUG='-O0 -g1'
cmake --build /home/rich/temp/copperfin-sqldisconnect-handle-5611-build --target test_prg_engine_numeric_behavior test_prg_engine_sql_cursors_metadata test_prg_engine_arrays test_prg_engine_string_math_functions -j4
ctest --test-dir /home/rich/temp/copperfin-sqldisconnect-handle-5611-build -R '^test_prg_engine_(numeric_behavior|sql_cursors_metadata|arrays|string_math_functions)$' --output-on-failure
cmake -S . -B /home/rich/temp/copperfin-sqldisconnect-handle-5611-sanitize -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_CXX_FLAGS_DEBUG='-O0 -g1 -fsanitize=address,undefined,float-cast-overflow -fno-omit-frame-pointer' -DCMAKE_EXE_LINKER_FLAGS='-fsanitize=address,undefined,float-cast-overflow'
cmake --build /home/rich/temp/copperfin-sqldisconnect-handle-5611-sanitize --target test_prg_engine_numeric_behavior test_prg_engine_sql_cursors_metadata test_prg_engine_arrays test_prg_engine_string_math_functions -j4
ctest --test-dir /home/rich/temp/copperfin-sqldisconnect-handle-5611-sanitize -R '^test_prg_engine_(numeric_behavior|sql_cursors_metadata|arrays|string_math_functions)$' --output-on-failure
ctest --test-dir /home/rich/temp/copperfin-sqldisconnect-handle-5611-build -R '^test_(changelog_fragment_assembler|changelog_fragments_valid|contributor_signoff_contract|agent_channel_contract)$' --output-on-failure
```

VR-5611-SQLDISCONNECT-REGRESSION-001: original expression dispatch fails
exactly 41 new conversion/caught-message assertions in GCC (numeric 6.40s;
full four-suite run 8.22s) and Clang sanitizers (numeric 29.95s; full run
36.92s). All three direct operation helpers, preserved callback/coercion
controls, existing GET/SETPROP rows and three neighbors pass before dispatch
changes. No baseline sanitizer diagnostic was produced.
After changing only SQLDISCONNECT's first-argument dispatch and using the
existing safe Numeric diagnostic formatter, GCC passes 4/4 in 9.22s
(numeric 7.23s). Changelog assembly/fragment, contributor-signoff and
live-channel contracts pass 4/4 in 4.21s; all 144 fragments validate.
VR-5611-SQLDISCONNECT-SANITIZER-001: final Clang ASan/UBSan/float-cast-overflow
passes 4/4 in 38.78s (numeric 31.45s) without sanitizer diagnostics.

Runtime tests ran serially across build directories. Fixture identities and
git diff --check pass. Hosted exact-head checks/review remain separate
integration gates. Local evidence is not independent-human, installed-GUI,
native connected SQL or full SQLDISCONNECT/product parity evidence.

## Documentation assurance

DQ-5611-SQLDISCONNECT-CONVERSION-001 requires documented admission,
negative-only compatibility aliases, lifecycle/type boundaries, serial
reproduction and rollback.
DV-5611-SQLDISCONNECT-CONVERSION-001: maintainer-authorized agent self-review
verified fixture identities, independent expectations, error-before-disconnect
walkthrough and completed automated results above before shipping.
No independent-human verification is claimed.

Procedural delta: native reproduction runs in a fresh connection-free process;
local synthetic normal/sanitizer cases use fresh session handle 1, compare
removal and preserved metadata, reset mode and clean the connection. Verification
covers conversion plus three neighbors without ODBC/network, VM, installer
or system changes.
Misuse severity is medium under HZ-runtime-crash-01/HZ-data-corruption-01:
do not infer repaired disconnect-all/absent/type behavior, native lifecycle
parity or safety of other SQL callers from these state comparisons.
Walkthrough outcome: rejected conversion preserves handle 1 and its connect
action; admitted handle-1 aliases remove it, mode reset remains observable and
cleanup leaves no synthetic connection. The successful-disconnect event remains
after the unchanged callback result, unreachable on conversion rejection.
Rollback reverts only this conversion while retaining independent unchecked-
path regressions. Incorrect guidance requires correcting the matrix/handoff/
release-PR evidence and notifying the owner before reuse.
