# ItemData Numeric read-index recovery

Pre-helper/pre-assertion/pre-configuration mapping: 2026-10-11T01:22Z,
admitted OPEN/rhamenator/agent-approved #5611/#6776, main
24f796c6a3ea84396f4d99fd76529ad50c7fc5c0. Existing code is verification
context, never a requirement source. This section records the pre-code mapping;
completed originals and subsequent migration are recorded in baseline-audit.md.

## Independent allowed evidence

Installed VFP9 09.00.0000.7423 under Wine/private Xvfb, no VM/system change.
Every case creates a fresh ListBox or ComboBox, adds three rows, initializes
ItemData101/202/303 and selects row2. Initialization avoids the documented
uninitialized AddItem values. Returned values/errors and count3/selection2/
payload101,202,303 are captured after each caught call.

Two complete98-line/96-call probe observations match, native-1.out/native-2.out:
SHA-256 8d00c7ee5801837d2094c976f8413768c6a639d026cace580ed632f50c397ef2.
Two complete42-line/40-call boundary observations match, boundary-1.out/
boundary-2.out: e1d728b5a10e8d6b8079c00a90a65a3a696813c48ecab307988d072ea0288bfd.
probe.prg SHA b1e346c9cff48ad6618fcac79bce6b23b02ae8b0123378607491c9a2e58ddb14;
boundary.prg SHA f618001fc788bb7c27c21562360e67545446654ff43877392e971b4bf46b52e0.
Four serial runs completed0/harvested before the next, full wrapper logs in
ignored build/itemdata-native. No excluded native run or GUI qualification.
Generated FXPs moved to ignored scratch, not read/decompiled/committed.

Existing /home/rich/bin/vfp9-probe was read completely, snapshotted unedited
as runner-source.txt and used unchanged,
SHA 1e64d290b8994acf0ad10eb3448b42dc72a6a109ab436b955a59587e9c1abb06.
It generates temporary SCREEN=OFF/RESOURCE=OFF config and TRY/DO/CATCH/QUIT
harness, private xvfb-run, Wine prefix /home/rich/copperfin-wine-vfp9 and
drive_c/hotfix968409/vfp9.exe,25-second deadline/owned child cleanup. Actual
temporary harness/config paths are deleted by the runner, not retained or
represented as committed files. Sources retain actual absolute output paths.
Reproduce serially with vfp9-probe <program.prg> --result <source-output-path>,
adapt only task-owned absolute paths, preserve raws before repeating and
harvest each job. Never run on the desktop/start VMs during backup reset.

Shipped Microsoft ItemData help fully read at
/home/rich/temp/vfp9-help-len/html/df63792f-562d-4792-b5b9-bf1178e7ed1d.htm,
SHA 3dab08e695f79b4e2f27edaca71d303594ff157be3c1062ca67fbd9e99b80366.
It documents runtime indexed Numeric array for ComboBox/ListBox, display-order
index and uninitialized AddItem values, not fractional/range/error mechanics.

Both classes agree:0/.49/.5/.9 and negative sub-units return0;
1/1.49/1.5/1.9=>101,2/2.49/2.5/2.9=>202,3/3.49/3.5/3.9=>303,4=>0.
-1/-1.9=>31. INT32_MAX=>0 but INT32_MAX+.1/.5/.9=>31. Positive2147483648,
4294967295/6/7/8/9, huge finite and +Infinity=>31. Negative-2147483648 and
sub-unit fractions=>31; -2147483649 and fractions=>0; -4294967295.9=>101,
-4294967294.5=>202,-4294967293=>303,-4294967296/.5=>0,-4294967297/.9=>31.
Negative huge finite/-Infinity=>0. +/-1e-300=>0. Native logical/NULL/Character/
Currency controls=>31, retained but outside selected type-policy changes.
Near-one text0.9999999999999999=>101 is native textual evaluation evidence,
not proof of identical binary representation or global decimal parsing parity.
Binary-nextafter/NaN/exact64 requirements below are explicitly parent-derived.

## Requirement, architecture and verification mapped before code

RQ-CF-PRG-ITEMDATA-INDEX-NUMERIC-001 derives from
RQ-CF-PRG-NUMERIC-BEHAVIOR-001, owner #5611/#6776 and extension intent2026-10-06,
native/help evidence and HZ-runtime-crash-01/HZ-data-corruption-01.
Default COPPERFIN truncates finite Numeric toward zero and admits converted
0..INT32_MAX; reject nonfinite/negative/out-of-domain with localized catchable31,
no wrap/clamp. INT32_MAX fractions below2147483648 are admitted by default:
explicit derived consistent truncation, NOT native raw-ceiling parity.
Admitted0 or beyond actual row count returns Numeric0; existing row ItemData
is otherwise returned unchanged. Exact signed/unsigned64 never pass via Double.

Explicit VFP9 first checks observed positive RAW INT32_MAX ceiling, then uses
parent-defined signed-low32 conversion; negative converted index rejects31,
zero/positive admit. Negative huge/-Infinity indefinite0 is observed; NaN
rejects31 as parent-derived containment, not native evidence. Exact64 raw
comparison/low32 is derived extension policy, not VFP9 syntax.
Conversion rejection precedes indexed lookup, destination assignment and method
action/success publication. Row data/count/selection stay unchanged in the
focused stable RowSource0 fixtures. Existing upstream selection-signature
materialization and property delegates/accessors remain unchanged; arbitrary
callbacks/materialization are not qualified for whole-call atomicity.
No index-sized allocation/iteration is introduced.

Expression ItemData(expr)/ItemData[expr] first attempt a formatted textual
selector in prg_engine_expression.inl; integer formatting can bypass the method
arm and fractional formatting can lose precision. Further read-only route
trace01:31Z found postfix (oList).ItemData[expr] has a third formatted fast
path; postfix parentheses already invoke the method callback. The final
pre-migration plan in baseline-audit.md preserves the raw single selector
through the private expression native-read callback and existing core receiver
resolver to reflection's ItemData position. It preserves property-first
visibility/accessor/metadata ordering and avoids duplicate path resolution.
One shared typed native reader also serves invoke_native_list_control_method's
itemdata arm in prg_engine_runtime_surface_object_methods.inl. Current-session
mode is read at reflection and method dispatch; the old method overload remains
for existing direct callers. Distinct checked_itemdata_index_numeric_argument
in helpers.h/.cpp was UNUSED through BOTH complete retained originals. Use
existing localized Array.Error.SubscriptOutOfRange text with explicit31;
do not change unrelated array1234 mapping. Textual GETPEM/reflection selectors
already use checked invariant integer parsing, not evaluated Numeric, and
stay unchanged. Writes/payload, List/Selected/other methods, materialization/
lifecycle/allocation, arity, other types/NULL/Currency/consumers are excluded.

VR-5611-ITEMDATA-INDEX-NUMERIC-001: independent literal both-mode direct
Numeric/exact/NaN/infinity/nextafter/subnormal boundaries/unsupported adapter
guards. VR-5611-ITEMDATA-INDEX-NUMERIC-002: fresh both-mode/two-session public
shards for BOTH classes and parentheses/brackets: result/kind,31/sentinel/
continuation, unchanged count/selection/row data, cursor/mode/neighbor/reset,
four locales and preserved types/textual/List/Selected controls.
Final refrozen188 direct/four368 public counters include bounded postfix
parentheses/brackets. The preliminary compile failure is not a complete original;
the explicit refreeze and BOTH completed originals are retained in baseline-audit.
The first migration additionally exposed a fixture receiver that never reaches
postfix parsing: grouped (oList) returns immediately. Separate recovery gap
[#7139](https://github.com/rhamenator/Project-Copperfin/issues/7139) leaves native
grouped syntax unverified; this slice does not change grammar. Second explicit
refreeze uses IIF(.T.,oList,.NULL.) with the same eight boundaries/expectations/
assertions/counters and repeats BOTH originals before reapplying migration.
Necessary native type-control differences are separately recorded in
[#7138](https://github.com/rhamenator/Project-Copperfin/issues/7138); they remain
out of scope and are not native type parity or new work admission.
Five helper/test/config hashes frozen BEFORE configure; BOTH full original GNU
and Clang sanitizer raws retained BEFORE migration. Any frozen input/assertion/
helper/config repair requires refreeze and BOTH originals repeated. Then
fixed focused, unchanged runtime-surface/numeric/exact/localization/isolation/
contracts and broad hosted exact-head evidence/actual clean review required.
Do not infer sanitizer findings from uninstrumented llround.

## Documentation assurance

DQ-5611-ITEMDATA-INDEX-NUMERIC-001 requires mode/range/error/exact semantics,
read/write/textual-selector/native/derived boundaries, reproduction/rollback.
DV-5611-ITEMDATA-INDEX-NUMERIC-001 verifies retained native and planned bounded
walkthrough. HZ-runtime-crash-01, HZ-data-corruption-01 and HZ-doc-command-01
retain registered severity/status. Bounded documentation misuse medium:
aliasing may display another item's metadata; no persistence writes here or
downstream application safety qualification. Procedural delta:1.9=>row1
instead of half-away row2; -1/oversized modern indices=>catchable31 rather
than accidental lookup/empty result. Only explicit VFP9 admits negative aliases.
Planned walkthrough BOTH classes/routes:1.9=>101,4=>0,-1=>31;
-4294967295=>modern31/legacy101;INT32_MAX+.5=>modern0/legacy31;
+Infinity=>31,-Infinity=>modern31/legacy0; locale/context/row atomicity.
Development maintainer self-review confirms the independent native/derived
distinction, repaired receiver provenance and selected-only typed route. GNU
fixed13/13 and Clang sanitizer13/13 verify the bounded walkthrough; full raw
outputs/hashes and honest preliminary failures/refreezes are in baseline-audit.
Documentation contracts5/5 pass (precommit source/checkpoint limitations are
explicit in the audit); hosted exact-head regression/review remains pending. This is not
independent-human signoff or platform/leak/application qualification.
Rollback reproduced false boundaries by reverting this bounded slice;
notify callers of exact input/mode/revision and retest saved scripts, not
weaken assertions or silently enable legacy mode. No new KBX/parent closure/
release/type/GUI/complete-control parity claim.
