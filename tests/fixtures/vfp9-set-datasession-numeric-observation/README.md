# Installed VFP9 SET DATASESSION numeric observations

Recovered 2026-10-06 from installed Visual FoxPro 09.00.0000.7423 under Wine
using the unchanged owner-installed /home/rich/bin/vfp9-probe runner.
datasession.prg is complete clean-room source; datasession.out retains VERSION,
a private Session object at ID 2, 61 selection commands and final session 1.
Two fresh final runs match all 64 output lines byte-for-byte. Earlier fresh
runs matched the 54-command subset before adding seven from-session-2 cases.
No persistent table, SQL/backend/network connection, VM, installer or system
change was used. Private session lifetime is confined to the probe process
and explicitly released at the end. Generated FXP is ignored; no proprietary
binary was inspected.

## Selected requirement and boundary

RQ-CF-PRG-SET-DATASESSION-NUMERIC-001 derives from owner-approved #5611/#6776,
RQ-CF-PRG-NUMERIC-BEHAVIOR-001 and HZ-runtime-crash-01/HZ-data-corruption-01.
Only SET DATASESSION TO's llround/int selector dispatch is selected.
Ordinary Numeric fractions truncate toward zero. Default COPPERFIN requires
finite conversion to a positive signed-int32 selector. Explicit VFP9 preserves
signed-low-32 aliases before that positive domain; converted zero/negative
selectors still reject. Exact int64/uint64 avoid double rounding. NaN rejects
both modes as derived safety policy, not a native NaN observation.
Other existing coercions retain checked half-away rounding/minimum-1 clamping,
without native type-parity claims.

Native live sessions 1 and 2 distinguish fractions and positive/negative
32-bit aliases, including 4294967297 selecting 1 and -4294967294 selecting 2.
Invalid selectors raise 1540 and preserve the previous active session, from
either 1 or 2. Other positive indices cannot be independently distinguished
in this two-session native fixture; their conversion follows documented
checked signed-int32/shared low-32 policy, not a claim of native existence.
Numeric rejection raises localized catchable 1540 before active-session
mutation, state creation or a successful runtime.datasession event.
Diagnostic wording is Copperfin catalog policy, not native error-text parity.

Session existence/creation/lifecycle, other operand type admission and other
setters remain outside scope. Native missing session 3 raises 1540;
Copperfin's existing creation of positive IDs is preserved. Native Currency
$0.5 and Logical/NULL/Character controls raise 10, and $1.5 truncates to 1;
existing Copperfin coercions are preserved separately. This encountered gap
is #7003, filed against exact main 667b6fd61936a65b5a5944aa048f92e6947bf0d1
without implementation admission. No argument-sized allocation or loop is
introduced: an admitted positive ID reaches the existing one-state lookup.

## Reproduction and identity

From the repository root:

```sh
/home/rich/bin/vfp9-probe tests/fixtures/vfp9-set-datasession-numeric-observation/datasession.prg
sha256sum tests/fixtures/vfp9-set-datasession-numeric-observation/datasession.prg tests/fixtures/vfp9-set-datasession-numeric-observation/datasession.out
```

Source SHA-256: 70c35553297b702ea3fa4eb48bc35b3faacee297c01f8da0c360f71e2cf26054.
Output SHA-256: 6239f2ed3366fc65b486741b7e6012ed57d18048bd230174fd6dbe00059bedab.
VR-5611-DATASESSION-NATIVE-001: two fresh final runs match all 64 lines,
including 61 commands, live private ID 2 and final session 1.

## Focused verification

140 direct calls cover the independent Numeric table, adjacent doubles,
exact extended integers, NaN and preserved checked coercions.
108 fresh synthetic script cases compare catchable error, selected/preserved
session, session-local DELETED settings, retained session-2 state, mode reset
and successful-event count. Two resumable-operand cases verify truncation
and rejection after a UDF changes the current session. Four locale cases
verify caught text/code and preserved active session in every shipped locale.
Numeric suites run serially across build directories because they share
temporary paths. Normal and sanitizer directories are isolated.

```sh
cmake -S . -B /home/rich/temp/copperfin-set-datasession-5611-build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS_DEBUG='-O0 -g1'
cmake --build /home/rich/temp/copperfin-set-datasession-5611-build --target test_prg_engine_numeric_behavior test_prg_engine_relations test_prg_engine_database_lifecycle test_prg_engine_date_time_functions test_localization -j4
ctest --test-dir /home/rich/temp/copperfin-set-datasession-5611-build -R '^test_prg_engine_(numeric_behavior|database_lifecycle|date_time_functions|relations)$' --output-on-failure
cmake -S . -B /home/rich/temp/copperfin-set-datasession-5611-sanitize -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_CXX_FLAGS_DEBUG='-O0 -g1 -fsanitize=address,undefined,float-cast-overflow -fno-omit-frame-pointer' -DCMAKE_EXE_LINKER_FLAGS='-fsanitize=address,undefined,float-cast-overflow'
cmake --build /home/rich/temp/copperfin-set-datasession-5611-sanitize --target test_prg_engine_numeric_behavior test_prg_engine_relations test_prg_engine_database_lifecycle test_prg_engine_date_time_functions -j4
ctest --test-dir /home/rich/temp/copperfin-set-datasession-5611-sanitize -R '^test_prg_engine_(numeric_behavior|database_lifecycle|date_time_functions|relations)$' --output-on-failure
ctest --test-dir /home/rich/temp/copperfin-set-datasession-5611-build -R '^test_(localization|locale_catalog_install_contract|native_test_isolation_contract|changelog_fragment_assembler|changelog_fragments_valid|contributor_signoff_contract|agent_channel_contract)$' --output-on-failure
```

VR-5611-DATASESSION-REGRESSION-001: the corrected snapshot tests against
the original selector dispatch fail exactly 251 selected assertions in GCC
(numeric 7.04s; full four-suite run 8.42s) and Clang sanitizers (numeric 34.27s;
full run 40.21s). Direct helper calls, existing numeric rows, preserved
non-Numeric coercion controls and all three neighbors pass before dispatch
changes. No baseline sanitizer diagnostic occurred. Initial test development
used a LOCAL not exposed in the runtime snapshot; that harness issue was
corrected before these retained baseline results.

After the selected dispatch change, GCC passes 4/4 in 8.97s
(numeric 7.49s). Catalog install/localization, test-isolation inventory,
changelog assembly/fragments, contributor signoff and live-channel contracts
pass 7/7 in 20.91s; all 145 fragments validate.
VR-5611-DATASESSION-SANITIZER-001: final Clang ASan/UBSan/float-cast-overflow
passes 4/4 in 39.54s (numeric 32.77s) without sanitizer diagnostics.
The test-isolation audit marks locale mutation scoped/restored and retains
serial scheduling. Normal/sanitizer numeric suites ran serially across build
directories. Fixture identities and git diff --check pass; exact-head hosted
checks/review remain separate integration gates.

The broad unchanged control-flow baseline was interrupted after sustained
execution before completing; no pass or failure is claimed for that run.
Focused resumable operands are explicitly tested, with neighboring relations,
database lifecycle and date-time suites used for completed local coverage.
Full hosted regression is a separate exact-head integration gate.

## Documentation assurance

DQ-5611-DATASESSION-CONVERSION-001 requires documented selector admission,
compatibility aliases, lifecycle/type limits, isolated serial reproduction
and rollback.
DV-5611-DATASESSION-CONVERSION-001: maintainer-authorized agent self-review
checked fixture identities, independent expectations, error-before-mutation
walkthrough and completed automated results above. No independent-human
verification is claimed.

Procedural delta: native reproduction creates only a temporary private Session
object; local synthetic cases begin in session 2 with DELETED ON, compare
selection/rejection and retained settings/events, reset modes and return to 1.
Misuse severity is medium under HZ-runtime-crash-01/HZ-data-corruption-01:
do not infer native session creation/lifecycle/type parity, installed-product
qualification or safety of other setters from this bounded conversion.
Walkthrough outcome: rejected conversion preserves session 2 and its setting,
emits no successful selection event, remains catchable and completes cleanup.
Valid truncation and admitted aliases select the expected state. Resumed
rejection preserves the post-expression session, not a stale pre-call session.
Rollback reverts only the selected conversion and catalog key while retaining
independent regressions. Incorrect guidance requires correcting the matrix,
handoff and release/PR evidence and notifying the owner before reuse.
