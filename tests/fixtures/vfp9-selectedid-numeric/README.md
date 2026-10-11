# SelectedID Numeric item-ID read requirements recovery

Pre-code mapping recorded 2026-10-11T06:18Z, BEFORE a SelectedID helper,
assertions, registrations, isolation or production edits. Admitted OPEN,
owner-authored exact-agent-approved #5611/#6776, base main
2cec530ddeb06abf70c7c30ffc83e8c04264c456. Only native ComboBox/ListBox
SelectedID first-argument Numeric/exact item-ID READ conversion is selected.
Writes, ID creation/mutation, lifecycle, other consumers, non-Numeric/NULL/
Currency/arity/general grammar and upstream materialization are excluded.

## Independent evidence

Installed Visual FoxPro 09.00.0000.7423 in existing private Wine prefix
/home/rich/copperfin-wine-vfp9; unchanged /home/rich/bin/vfp9-probe SHA256
1e64d290b8994acf0ad10eb3448b42dc72a6a109ab436b955a59587e9c1abb06.
Actual runner uses temporary harness/config, auto-deleted after each run;
runner-source.txt retains its bytes, not invented persistent harness files.
No VM start/recreation, desktop testing, credentials or binary decompilation.

Both classes, fresh object per call, stable count3/selection row2/data101,202,303.
Independent matching sparse-ID11,22,33 pair:96 lines/94 calls, SHA256
679f277bab55c1ed913ef1c9dab51c5be7fad9c19e75657f125c03614a691723.
22/22.49/22.5/22.9 return Logical True, 2 is not the selected item ID;
11/33/missing23/34 return False. Negative -4294967274 and -.9 alias22 True,
negative aliases11/33 False; -1/-1.9 error31. Positive raw INT32_MAX admitted
missing False, larger positive/huge/+Infinity error31. Negative huge/-Infinity
False. Logical/NULL/Character/Currency controls error31 natively, not permission
to change preserved Copperfin type policy in this slice.
Independent dense-ID1,2,3 pair:98 lines/96 calls, SHA256
de1188fe09751f386646da523b1db4c2573744b7af17c7def8b08b9e3f7dd506;
boundary pair:42 lines/40 calls, SHA256
9121677b4af3da963ae69ce0926399557c66e9702a56ca56a374365d559abdc2.
Sources explicitly call SelectedID; coincident raw hashes with prior Selected
observations are observed equality, not substitution of the prior oracle.
Dense 2.9 True/1.9 False, negative alias2 True, positive raw INT32_MAX+.1/.5/.9
error31; -2147483648/.9 and -4294967297/.9 error31, -2147483649/.9 missing
False, -4294967296/.5 zero False. Tiny finite/sub-units False. Native decimal
text is not proof of identical binary64 parsing; nextafter/subnormal cases are
independently parent-derived mathematical boundaries.

Shipped Microsoft SelectedID Property help, fully read:
/home/rich/temp/vfp9-help-len/html/84455548-8d3b-4239-8a78-731a98da7d48.htm
SHA25682625673e604d4dab286787181fc2ed6c76720ea70ca0531b3a9cf97456f2929.
It specifies item-ID addressing and Logical selected state for both classes;
write/event behavior is outside this read-only conversion slice.

Additional matching independent sparse identity boundary pair:22 lines/20 calls,
SHA256ee718035d84bf7b6d3a3d652e97fbb3a864ebf61d8c8f871db2b21065451253e.
Both 2/2.9 are False while22.9 is True with stable row2/ListItemID22;
21.9/23.9 False, negative alias22 fractional True and raw INT32_MAX+.5 error31.
These inputs were recovered before the initial freeze, not a test repair.

## Requirement and architecture before code

RQ-CF-PRG-SELECTEDID-NUMERIC-001 derives from admitted #5611/#6776,
RQ-CF-PRG-NUMERIC-BEHAVIOR-001, owner extension intent2026-10-06 and
HZ-runtime-crash-01/HZ-data-corruption-01. Default COPPERFIN finite Numeric
truncation toward zero followed by checked0..INT32_MAX; invalid/nonfinite inputs
raise localized catchable31 before item-ID lookup/destination assignment/success.
Exact signed/unsigned64 compare without Double. Converted0 or missing item ID
returns Logical False; an existing ID returns that row's bit, NOT row-number
addressing. Truncated INT32_MAX+.5 is default-admitted missing False, while
explicit VFP9 applies independently observed positive RAW INT32_MAX ceiling
then parent-defined signed-low32/nonnegative admission. Negative huge/-Infinity
indefinite0 observed; NaN rejection and exact64 projection are parent-derived
containment/extension policy, not native extended syntax. INT64_MIN+22 has
low32=22: sparse-ID22 True in VFP9/default31, distinguishing accidental Double
rounding to zero. INT64_MIN+2 similarly distinguishes the dense fixture.

Read-only route trace: expression parentheses/brackets try reflection first;
there is no SelectedID indexed reflection getter, so native method receives
raw PrgValue. Supported IIF-result postfix parentheses/brackets likewise retain
raw selector and current-session mode. Plan only the selectedid arm in
prg_engine_runtime_surface_object_methods.inl and a distinct
checked_selectedid_numeric_argument helper in helpers.h/.cpp. No parser or
reflection production edits. Textual GETPEM and existing direct-call overloads
remain unchanged. Existing materialize/sync order is preserved; stable RowSource0
state controls do not qualify arbitrary callbacks/upstream materialization
atomicity. No argument-sized allocation or iteration is introduced.

VR-5611-SELECTEDID-NUMERIC-001: literal both-mode direct Numeric, exact64,
NaN/infinity/adjacent/subnormal boundaries and adapter guards.
VR-5611-SELECTEDID-NUMERIC-002: fresh both-mode/two-session public cases,
both controls/four raw routes, dense and sparse IDs, independent results/kinds,
catchable31/locales, destination sentinel/continuation/context/cursor/reset,
unchanged count/data/bits/item keys/ListIndex/ListItemID/no external invoke event.
Preserved unsupported coercions are NOT native parity.
Necessary preserved type-admission discrepancy is separately recorded in
[#7145](https://github.com/rhamenator/Project-Copperfin/issues/7145) against
current main2cec530dd, OPEN/owner/no labels; no type-policy production change
or admission label is requested or applied by this slice.
Freeze helper/test/registration/isolation BEFORE configuring; distinct helper remains UNUSED
through BOTH full original GNU and Clang sanitizer runs. Retain complete raw
logs/status/hashes/counters BEFORE migration. Any frozen repair requires explicit
refreeze and BOTH originals repeated. Fixed normal/sanitizer neighbor,
documentation, hosted exact-head required checks, actual clean exact-head
Claude-FIRST/authorized fallback review and resolved conversations remain gates.

## Documentation assurance before code

DQ-5611-SELECTEDID-NUMERIC-001/DV-5611-SELECTEDID-NUMERIC-001 require mode/
range/error/exact/read-write/native-derived boundaries, controlled reproduction,
retained evidence and rollback. Active HZ-runtime-crash-01/HZ-data-corruption-01/
HZ-doc-command-01 retain registered severity/status. Bounded documentation misuse
is medium: aliased item IDs could misdirect later application actions, but this
slice performs no persistence writes or downstream safety qualification.
Procedural delta:22.9 selects item22 rather than old rounded23 False; modern
invalid negatives/nonfinite/oversized reject31, only explicit VFP9 admits negative
aliases. Walkthrough shall verify sparse22.9 True/2.9 False/-4294967274
modern31/VFP9 True, INT64_MIN+22 equivalent derived boundary, and dense1.9 False/
2.9 True/INT32_MAX+.5 modernFalse/VFP9 31; all controls/routes/locales/state.
Development delegated maintainer self-review plus applicable automation is
required, not independent-human or release qualification. Rollback: revert only
this bounded slice, rerun frozen neighbors, notify integrators via changelog
before re-release. No parent/family/KBX/hazard closure inferred.

## Controlled reproduction and preservation

The exact GNU/Clang configure, build and23-subject CTest commands are in
baseline-audit.md. Run serially in this dedicated slice checkout with its
owned700-mode build/selectedid-tmp directory, nice10/ionice2:7 and the disclosed
600s LOCAL diagnostic bound. This is not a CI timeout/performance policy.
Keep full failed as well as successful raw logs before changing production;
leaks are disabled in the sanitizer run, so no leak qualification follows.

Native reproduction uses only the existing installed Wine VFP9 environment
and retained runner. probe.prg, dense.prg, boundary.prg and identity.prg contain
explicit owned build/selectedid-native output paths. Inspect the exact path,
then run vfp9-probe <program.prg> --result <that exact output path> serially;
preserve each full output before a later run replaces it. The temporary
runner harness/config auto-delete; generated FXP is retained only as uninterpreted
scratch evidence, never decompiled or made a requirement source. These programs
write synthetic observations only. Do not start/recreate a local VM while
cleanup/backup-reset completion is unconfirmed; never copy credentials to logs.

preservation-driver.cpp.txt calls six existing, UNCHANGED list_controls_03
groups: SelectedID selection, removal, Selected writes/reads, MultiSelect,
Sorted and MoverBars. It adds no assertion or frozen-input/production change.
Compile as C++ with the corresponding compiler/sanitizer flags in the audit,
-Iinclude -Itests -Isrc/runtime, and -x c++ for the retained driver then -x none
for the existing test source/objects/archives. Link the same build's
prg_engine_test_support.cpp.o plus cf_xbase_runtime, cf_design_model,
cf_prg_analysis, cf_runtime_text, cf_vfp_assets, cf_platform_profile,
cf_security, cf_localization and cf_platform_support archives. Run serially
with COPPERFIN_LOCALE_DIR set to resources/locales and the same private TMP/
sanitizer environment. This is bounded preservation, not the full hosted
runtime-surface regression or arbitrary callback/materialization qualification.

## Fixed results and assurance boundary

Both full original23-subject runs completed and were copied/cmp/hash-checked
before the first production migration: each19/23 with only four new SelectedID
public failures, complete frozen/neighbor counters and zero other FAIL lines.
The same frozen inputs then pass GNU23/23/189.68s and Clang sanitizer23/23/
305.13s. All200direct/four476public SelectedID and unchanged Selected190/
four368, ItemData188/four368, GETCOLOR174/four80 counters complete; no fixed
assertion failure or sanitizer report, leaks disabled. Full unedited raws,
hashes and phase order are retained in baseline-audit.md.

Only the SelectedID Numeric/exact READ arm changed. Existing sync/materialize
order, other coercions, zero/missing-ID False, lookup, writes and other routes
are unchanged; unsupported coercions are preserved controls, not native parity.
No frozen helper/input/assertion/registration/isolation repair occurred.
Six unchanged preservation groups pass in each build, without assertion edits.
Bounded medium DQ/DV delegated development maintainer self-review and automated
walkthrough are complete; details and retained identities are in the audit.
Documentation contracts pass5/5 at the retained precommit checkpoint; exact
signed-head external checks/clean review/resolved conversations remain gates.
No family/parent/release/independent-human
or arbitrary-callback/whole-call allocation qualification follows these results.

Reproduce the five documentation contracts serially with the same private TMP,
ctest -V --output-on-failure --timeout1200 -j1 and exact five test names in
documentation-contracts.log. This local diagnostic budget does not change CI.
The audit records checkpoint hashes and later completion-only wording; the
precommit run is not presented as an exact-final-head documentation rerun.
