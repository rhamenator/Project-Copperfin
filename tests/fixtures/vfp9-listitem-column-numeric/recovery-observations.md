# Independent explicit-column READ recovery checkpoint

RQ-CF-PRG-LISTITEM-COLUMN-NUMERIC-001 / VR001+002 / DQ-DV001.
This is permitted-source observation evidence, not a completed requirement map,
implementation, expected conversion table, development assurance sign-off or
platform acceptance. Parent5611/6776 remain open. Ten selected main inputs are
still byte-identical; no helper/assertion/configuration/build/caller change.

## Allowed inputs and capture method

Completely read installed shipped ListItem Property, BINTOC Function and
ColumnCount Property help. Exact local paths and hashes are retained in
allowed-source-provenance.sha256, alongside installed VFP9 and the unchanged
runner. Proprietary help/executable contents are not copied into this repo.
ListItem documents stable-ID access, an optional column and Character return;
omitting a column is distinct from the supplied-column READ selected here.
ColumnCount's stated255 maximum concerns Grid, not ListBox/ComboBox. Neither
page supplies an explicit numeric-conversion algorithm or a READ capacity.
BINTOC BRS supplies the actual eight-byte Numeric representation, retained as
hex before and after each invocation.

The unchanged /home/rich/bin/vfp9-probe is retained byte-for-byte as
runner-snapshot.txt (data, not a new executable). It invokes installed
Visual FoxPro09.00.0000.7423 through the existing private Wine/Xvfb runner,
uses a25-second scoped process-group deadline and restores the previous
result on failure. No VM/start/clone, credentials, decompilation or backup-reset
completion claim. Another fixture was used ONLY for capture framing, never
operand vectors, results, numeric expectations, bounds or helper policy.

Each call creates a fresh native ListBox or ComboBox, populates stable IDs
11,22,33 at display rows1,2,3, distinct cell captions, ItemData101/202/303,
selected row2 and ListBox TopIndex2. Both target IDs11/22 are exercised, making
stable-ID versus display-row access observable. Actual class/ID/row, operand
spelling/type/BRS, phase, error/message or returned type/text, workarea/alias/
data-session context and sampled control state are captured. State includes
count/columns/RST/binding/new-ID/new-index/selection/display/value/top and each
row's ID/data/selected flag/nine cells; configuration captures also sample
columns255/256. This is sampled state, not all object properties or memory.

## Actual matching pairs

All six native invocations returned actual0 with empty captured native stderr.
Each first/repeat full raw result and runner stdout are byte-identical, including
CRLF; complete VERSION/READ/DONE framing, four class-ID contexts, actual Numeric
type/unchanged BRS, invocation reach, unchanged sampled state/context and fresh
setup are independently checked by audit-capture.cjs. The audit does NOT compute
or assert a column-conversion expectation. All three audits actually returned0.

| Probe | Rows per invocation | Complete raw SHA-256 |
| --- | --- | --- |
| independent-probe.prg |224 (56 spellings x4 contexts)|5557e0bbdbab4ea9ddec445c786d6affaa65750fe3f270b8eaaaf6a0e1db1341|
| boundary-probe.prg |188 (47 spellings x4 contexts)|f10b36cec37e408280e5ccf0fbfd4d0508bba2dfb70848ec171e2d2e12272d6f|
| configuration-probe.prg |360 (15 spellings x6 configurations x4 contexts)|0dadded89e16375219eb68ace3b311baae04fd4cd2e29d74c3ebe69053bdbeb4|

First/repeat raw/stdout/stderr/status are retained under
captures/{independent,boundary,configuration}/. Complete probe sources and
their own FXP outputs are in this fixture. Independent first FXP was not
separately snapshotted; its retained repeat FXP is not a retroactive first
snapshot. Both boundary FXPs and the configuration pre-repeat corrected copy/
repeat FXP are retained. No native bytecode is decompiled.
The first configuration invocation's subsequent FXP-copy command named the
wrong fixture directory and actually failed1; that exact diagnostic/status is
retained. Native invocation0/raw were unaffected. A corrected copy before the
repeat returned0. The wrapper's final0 is NOT proof that the first copy worked.

## Observations, not generalized algorithms

In the nine-column control, ordinary1.9/2.9/9.9 read cells1/2/9;10 and65537
return empty Character.65537 does NOT read cell1. Positive2147483647 returns
empty but2147483647.1 and2147483648 raise native31. Negative-4294967295.9
reads cell1,-4294967294.1 reads cell2 and-8589934591 reads cell1. Both signs of
1E20/1E100/1E300 raise31. These observations do not justify transplanting a
different consumer's low16/write limit, AddListItem's32745 bound, a global
capacity claim, native31-to-Copperfin error mapping or an unchecked cast.

Near-integer behavior needs further recovery before policy/helper authoring:
actual BRS3FEFFFFFFFFFFFFF (1-2^(-53)) reads cell1; values1-2^(-52),1-2^(-51)
and1-2^(-50) also read cell1, while1-1E-14 raises31. At2,2-2^(-52) and
2-2^(-51) read cell2 but2-1E-14 reads cell1. At9,9-2^(-49) reads cell9 while
9-1E-12 reads cell8. Thus simple unconditional binary truncation alone does
not describe these native observations. Do not silently drop these vectors,
change their actual bits or invent a tolerance/precision rule from current
Copperfin code. Determine the bounded compatibility rule using further allowed
observations; explicitly distinguish any owner-derived default behavior.

Configurations are set BEFORE population:1 default RST/binding/columns9;
2 explicit RST0/empty binding/columns9;3 explicit RST0/space-only binding/
columns9;4 columns0;5 columns1;6 columns256. All reach the READ consumer;
all three-column9 configurations agree. Columns0/1 retain cell1 and return
empty for2. Columns256 reads distinct cells32/33/255/256 and returns empty
for257/65537. This supports those actual configurations, NOT native maximum
capacity, arbitrary types/RST/bindings/assignments or all lifecycle routes.

## Remaining order and active late-CI dependency

Finish independent near-integer/width/native-source recovery and the pre-code
RQ/architecture/VR/DQ-DV/hazard/safe-original/extension map. Exact64, NaN and
non-native binary-value boundaries require named numeric-policy/hazard/owner
parents; native CAST or another consumer is not an extension oracle. Then
UNUSED helper, independently authored literals and fixed-only hostile vectors,
freeze/source-isolation/CMake, BOTH complete original compiler suites before
any wiring, followed by fixed/sanitizers/SAME-compiler preservation/contracts/
exact-head review/postpush/required checks. No acceptance gate is waived.

During this recovery, retained PR7187 Windows run38636352844 attempt1 finished
with numeric_behavior180.07s timeout and aggregate failure, not full platform
qualification. #7188 records actual610 attempted/609pass/one timeout and the
unchanged full-attempt retry. Exact evidence remains canonical
build/pr7187-late-native/ and its retained copy in this fixture's late-windows/
directory. Investigation/retry is the immediate validation dependency; this
native recovery remains uncancelled and resumes at the gaps above. No owner
input blocker, unrelated production mutation or VM-reset assumption.

Checkpoint verification: all ten selected inputs and all270 PR7187 fixed runtime/
test inputs actually match their unchanged manifests; release-licensing contract,
changelog222 fragments and Node audit syntax checks each return0. The unrelated
handoff suffix from literal PR #6926 onward retains SHA-256
b6b6ef2953b527fa84f7779489973ad82e5cf4632aff5736291cde71fd41bbf1 in both canonical
and slice worktrees. These are checkpoint checks, not either required future
original/fixed runtime suite or release qualification. Native CRLF and actual
hosted raw diagnostics are preserved, not whitespace-normalized.
